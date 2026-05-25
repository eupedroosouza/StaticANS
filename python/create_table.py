import argparse
import csv
import os.path
import struct
from pathlib import Path

import numpy as np
import yaml


class Table:
    def __init__(self, type: int, id: int, alphabet: dict[int, int]):
        self.type = type
        self.id = id
        self.alphabet = alphabet


class TableResult:
    def __init__(self, table: Table, states: dict[int, list[int]], bitstreams: dict[int, list[tuple[int, int]]]):
        self.table = table
        self.states = states
        self.bitstreams = bitstreams


def read_alphabet(filename: str) -> list[Table]:
    path = Path(filename)
    if not path.exists() or (not path.is_file()):
        raise FileNotFoundError(f"File {filename} does not exist")

    tables: list[Table] = []
    with(open(filename, "r", encoding="utf-8")) as file:
        configuration = yaml.safe_load(file) or {}

        tables_node = configuration["tables"]
        for node in tables_node:
            type = np.uint8(node["type"])
            id = np.uint(node["id"])
            alphabet: dict[int, int] = {}
            alphabet_node = node["alphabet"]
            for symbol_node in alphabet_node:
                for k, v in symbol_node.items():
                    alphabet[int(k)] = int(v)
            tables.append(Table(type, id, alphabet))
            pass
        # for table_name, node in tables.items():
        #     alphabet: dict[int, int] = {}
        #     alphabet_node = node["alphabet"]
        #     for symbol_node in alphabet_node:
        #         for k, v in symbol_node.items():
        #             alphabet[int(k)] = int(v)
        #     tables[table_name] = Table(table_name, alphabet)
        #     pass

    return tables


def create_table(table: Table) -> TableResult:
    total = np.sum(list(table.alphabet.values())).astype(np.int32)
    cumulative = np.insert(np.cumsum(list(table.alphabet.values())), 0, 0).astype(np.int32).tolist()

    output_states, output_bitstreams = create_encoder_table(total, table.alphabet, cumulative)

    # print(output_states)
    # print(output_bitstreams)

    print(f"Generated table {table.type}/{table.id}")
    return TableResult(table, output_states, output_bitstreams)


def save_table_as_csv(table_result: TableResult, dir_csv: Path):
    filename = Path(os.path.join(dir_csv, f"{table_result.table.type}_{table_result.table.id}.csv"))

    if filename.exists():
        raise FileExistsError(f"File {filename} already exists")

    fields = ["state"]
    for key in table_result.table.alphabet.keys():
        fields.append(str(key))
    fields.append("")
    fields.append("state")
    for key in table_result.table.alphabet.keys():
        fields.append("size")
        fields.append(str(key))

    rows: list[list[str]] = []

    for state in table_result.states.keys():
        row: list[str] = [str(state)]
        for s in table_result.states[state]:
            row.append(str(s))
        row.append("")
        row.append(str(state))
        for bs in table_result.bitstreams[state]:
            row.append(f"{bs[0]}")
            if bs[0] == 0:
                row.append("")
            else:
                row.append(format(f"{bs[1]:b}"))

        rows.append(row)

    with open(filename, "w", encoding="utf-8", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(fields)
        writer.writerows(rows)

        print(f"Saved table {table_result.table.type}/{table_result.table.id} to CSV file: {filename}.")
    pass


def save_tables_as_binary(results: list[TableResult], bin_file: Path | str):
    path = Path(bin_file)
    if path.exists():
        raise FileExistsError(f"File {bin_file} already exists")

    with open(path, "wb") as file:

        file.write(struct.pack("<I", len(results)))

        for table in results:
            if not table.states or not table.bitstreams:
                raise RuntimeError(f"Table {table.table.type}/${table.table.id} is empty.")

            file.write(struct.pack("<B", table.table.type))
            file.write(struct.pack("<B", table.table.id))

            first_state_key = next(iter(table.states))
            symbols_size = len(table.states[first_state_key])
            file.write(struct.pack("<B", symbols_size))

            total = sum(table.table.alphabet.values())
            file.write(struct.pack("<H", total))

            for state, next_states in table.states.items():
                bitstream = table.bitstreams[state]
                for i, next_state in enumerate(next_states):
                    file.write(struct.pack("<H", next_state))
                    bs = bitstream[i]
                    file.write(struct.pack("<B", bs[0]))
                    file.write(struct.pack("<B", bs[1]))

    print(f"Saved binary tables on {bin_file}.")


# tANS
def create_encoder_table(total: int, alphabet: dict[int, int], cumulative: list[int]) -> tuple[
    dict[int, list[int]], dict[int, list[tuple[int, int]]]]:
    output_states: dict[int, list[int]] = {}
    output_bitstreams: dict[int, list[tuple[int, int]]] = {}
    r_max = (2 * total)
    for i_state in range(total, r_max, 1):
        symbols_states: list[int] = []
        symbols_bitstreams: list[tuple[int, int]] = []
        for s, s_freq in alphabet.items():

            state = i_state
            bitstream_size: int = 0
            bitstream: int = 0

            while state >= (2 * alphabet[s]):
                rem = state % 2
                bitstream = bitstream | (rem << bitstream_size)
                bitstream_size += 1
                state = int(state / 2)

            state = encode_rANS(total, cumulative, alphabet, s, state)

            symbols_states.append(state)
            symbols_bitstreams.append((bitstream_size, bitstream))
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
            state = int(state / 2)

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

    args = parser.parse_args()
    tables: list[Table] = read_alphabet(args.alphabet)
    if tables is None:
        return

    dir_csv: Path | None = None
    if args.csv is not None:
        dir_csv = Path(args.csv)
        if dir_csv is None:
            return
        if dir_csv.exists() and not dir_csv.is_dir():
            raise RuntimeError(f"{dir_csv} is not a directory")
        if not dir_csv.exists():
            dir_csv.mkdir(parents=True)
        pass

    res: list[TableResult] = []
    results: dict[Table, TableResult] = {}
    for table in tables:
        result = create_table(table)
        results[table] = result
        res.append(result)
        if not dir_csv is None:
            save_table_as_csv(result, dir_csv)

    if args.bin is not None:
        bin_file = Path(args.bin)
        save_tables_as_binary(res, bin_file)
        pass

    print("Done.")

    pass


if __name__ == "__main__":
    main()
