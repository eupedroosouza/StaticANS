import argparse
import csv
import os.path
from pathlib import Path

import numpy as np
import yaml


class Table:
    def __init__(self, name: str, alphabet: dict[int, int]):
        self.name = name
        self.alphabet = alphabet


class TableResult:
    def __init__(self, table: Table, states: dict[int, list[int]], bitstreams: dict[int, list[list[int]]]):
        self.table = table
        self.states = states
        self.bitstreams = bitstreams


def read_alphabet(filename: str) -> dict[str, Table]:
    path = Path(filename)
    if not path.exists() or (not path.is_file()):
        raise FileNotFoundError(f"File {filename} does not exist")

    tables: dict[str, Table] = {}
    with(open(filename, "r", encoding="utf-8")) as file:
        configuration = yaml.safe_load(file) or {}

        tables = configuration["tables"]
        for table_name, node in tables.items():
            alphabet: dict[int, int] = {}
            alphabet_node = node["alphabet"]
            for symbol_node in alphabet_node:
                for k, v in symbol_node.items():
                    alphabet[int(k)] = int(v)
            tables[table_name] = Table(table_name, alphabet)
            pass

    return tables


def create_table(table: Table) -> TableResult:
    total = np.sum(list(table.alphabet.values())).astype(np.int32)
    cumulative = np.insert(np.cumsum(list(table.alphabet.values())), 0, 0).astype(np.int32).tolist()

    output_states, output_bitstreams = create_encoder_table(total, table.alphabet, cumulative)

    # print(output_states)
    # print(output_bitstreams)

    print(f"Generated table {table.name}")
    return TableResult(table, output_states, output_bitstreams)


def save_table_as_csv(table_result: TableResult, dir_csv: Path):
    filename = Path(os.path.join(dir_csv, f"{table_result.table.name}.csv"))

    if filename.exists():
        raise FileExistsError(f"File {filename} already exists")

    fields = ["state"]
    for key in table_result.table.alphabet.keys():
        fields.append(str(key))
    fields.append("")
    fields.append("state")
    for key in table_result.table.alphabet.keys():
        fields.append(str(key))

    rows: list[list[str]] = []

    for state in table_result.states.keys():
        row: list[str] = [str(state)]
        for s in table_result.states[state]:
            row.append(str(s))
        row.append("")
        row.append(str(state))
        for bs in table_result.bitstreams[state]:
            stringed_bs = ""
            for b in bs:
                stringed_bs += str(b)
            row.append(stringed_bs)
        rows.append(row)

    with open(filename, "w", encoding="utf-8", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(fields)
        writer.writerows(rows)

        print(f"Saved table {table_result.table.name} to CSV file: {filename}.")
    pass


# The format
# [rows - int32][columns - int32][states... int32][bitstreams... int32]
def save_table_as_binary(table_result: TableResult, bin_file: Path):
    pass
    # path = Path(out_filename)
    # if path.exists():
    #     raise FileNotFoundError(f"File {out_filename} already exists")
    #
    # rows = np.sum(list(states.keys())).astype(np.int32)
    # columns = np.sum(list(alphabet.keys())).astype(np.int32)
    #
    # flat_states = [item for sublist in states.values() for item in sublist]
    # flat_bitstreams = [item for lists in bitstreams.values() for sublist in lists for item in sublist]
    #
    # barr = bytearray(8 + (2 * (rows * columns * 8)))
    # offset = 0
    # struct.pack_into("<II", barr, 0, rows, columns)
    # offset += 8
    #
    # for row in range(rows):
    #     for column in range(columns):
    #         data = states[row][column]


def generate_table_as_cpp_maps(table_result: TableResult) -> str:
    cpp_code = []
    name = table_result.table.name
    prefix = f"{name}_"

    states_def = f"const std::map<uint16_t, std::map<int8_t, uint16_t> > {prefix}states = {{\n"
    for state, next_states in table_result.states.items():
        # Transform the Python list on [next_0, next_1] em {{0, next_0}, {1, next_1}}
        inner_pairs = []
        for symbol, next_state in enumerate(next_states):
            inner_pairs.append(f"{{{symbol}, {next_state}}}")

        states_def += f"    {{{state}, {{{', '.join(inner_pairs)}}}}},\n"
    states_def += "};\n"
    cpp_code.append(states_def)

    bits_def = f"const std::map<uint16_t, std::map<int8_t, std::vector<uint8_t> > > {prefix}bitstreams = {{\n"
    for state, symbol_bits in table_result.bitstreams.items():
        inner_pairs = []
        for symbol, bits in enumerate(symbol_bits):
            # Transform the list of bits [1, 0]  on string {1, 0}
            bits_str = ", ".join(str(b) for b in bits)
            inner_pairs.append(f"{{{symbol}, {{{bits_str}}}}}")

        bits_def += f"    {{{state}, {{{', '.join(inner_pairs)}}}}},\n"
    bits_def += "};\n"
    cpp_code.append(bits_def)

    table_instantiation = f"Table {name}({prefix}states, {prefix}bitstreams);\n"
    cpp_code.append(table_instantiation)

    return "\n".join(cpp_code)


# tANS
def create_encoder_table(total: int, alphabet: dict[int, int], cumulative: list[int]) -> tuple[
    dict[int, list[int]], dict[int, list[list[int]]]]:
    output_states: dict[int, list[int]] = {}
    output_bitstreams: dict[int, list[list[int]]] = {}
    r_max = (2 * total)
    for i_state in range(total, r_max, 1):
        symbols_states: list[int] = []
        symbols_bitstreams: list[list[int]] = []
        for s, s_freq in alphabet.items():

            state = i_state
            bitstream: list[int] = []

            while state >= (2 * alphabet[s]):
                bitstream.append(state % 2)
                state = int(state / 2)

            state = encode_rANS(total, cumulative, alphabet, s, state)

            symbols_states.append(state)
            symbols_bitstreams.append(bitstream)
        output_states[i_state] = symbols_states
        output_bitstreams[i_state] = symbols_bitstreams

    return output_states, output_bitstreams


# Streaming-rANS

# That function encode all symbols input
def streaming_encode_rANS(total: int, cumulative: list[int], alphabet: dict[int, int], s_input: list[int]) -> tuple[
    int, list[int]]:
    state = total
    bitstream = []

    for s in s_input:
        while state >= (2 * alphabet[s]):
            bitstream.append(state % 2)
            state = state / 2

        state = encode_rANS(total, cumulative, alphabet, s, state)  # The rANS encoding step

    return state, bitstream


# That function decode one symbol input ant return state to decode others
def streaming_decode_rANS(total: int, cumulative: list[int], alphabet: dict[int, int], final_state: int,
                          bitstream: list[int], ):
    s_decoded, state = decode_rANS(total, cumulative, alphabet, final_state)

    while state < total:
        bit = bitstream.pop()
        state = (state * 2) + bit

    return s_decoded, state


# rANS
def encode_rANS(total: int, cumulative: list[int], alphabet: dict[int, int], s: int, state: int) -> int:
    s_freq = alphabet[s]  # current symbol count/frequency
    next_state = (int((state // s_freq)) * total) + cumulative[s] + (state % s_freq)  # get rANS next state
    return next_state


def cumulative_inverse(cumulative: list[int], pos: int) -> int | None:
    # use that to optimize the search of cumulative based on position with binary search
    idx = np.searchsorted(cumulative, pos, side="right").astype(np.int32) - 1
    if 0 <= idx < len(cumulative):
        return idx
    return None


def decode_rANS(total: int, cumulative: list[int], alphabet: dict[int, int], state: int) -> tuple[int, int]:
    pos = state % total
    s = cumulative_inverse(cumulative, pos)
    s_freq = alphabet[s]
    prev_state = (int((state / total)) * s_freq) + pos - cumulative[s]  # use int(x) to avoid a float

    return s, prev_state


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--alphabet", required=True, help="Alphabet file")
    parser.add_argument("--csv", required=False, help="CSV save directory")
    parser.add_argument("--bin", required=False, help="Binary (.bin) save file")
    parser.add_argument("--c", required=False, help="C (.h) save file")

    args = parser.parse_args()
    tables: dict[str, Table] = read_alphabet(args.alphabet)
    if tables is None:
        return

    dir_csv: Path | None = None
    if args.csv is not None:
        dir_csv = Path(args.csv)
        if dir_csv.exists() and not dir_csv.is_dir():
            raise RuntimeError(f"{dir_csv} is not a directory")
        if not dir_csv.exists():
            dir_csv.mkdir(parents=True)
        pass

    results: dict[Table, TableResult] = {}
    for table_name, table in tables.items():
        result = create_table(table)
        results[table] = result
        if not dir_csv is None:
            save_table_as_csv(result, dir_csv)
    print("Done.")

    bin_file: Path | None = None
    if args.bin is not None:
        bin_file = Path(args.bin)
        if bin_file.exists():
            raise RuntimeError(f"{bin_file} already exists")
        pass

    if args.c is not None:
        c_file = Path(args.c)
        if c_file.exists():
            raise RuntimeError(f"{bin_file} already exists")

        final_string = "\n"
        for table, result in results.items():
            final_string += generate_table_as_cpp_maps(result)
            final_string += "\n"
            pass
        final_string += "\n"

        with open(c_file, "w", encoding="utf-8") as f:
            f.write(final_string)

    pass


if __name__ == "__main__":
    main()
