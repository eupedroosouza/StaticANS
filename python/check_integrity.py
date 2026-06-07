import hashlib
import os
import re
import argparse

def extract_tensors(list_path):
    tensors = {}
    if not os.path.exists(list_path):
        print(f"Error: List file '{list_path}' not found.")
        return None

    with open(list_path, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            match = re.match(r'^(\d+)\s+(tensor_\d+\.bin)', line)
            if match:
                idx = int(match.group(1))
                filename = match.group(2)
                tensors[idx] = filename
    return tensors

def calculate_md5(file_path):
    if not os.path.exists(file_path):
        return None

    md5_hash = hashlib.md5()
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            md5_hash.update(chunk)
    return md5_hash.hexdigest()

def compare_directories(dir1, dir2):
    list1_path = os.path.join(dir1, "decoded_tensors.meta")
    list2_path = os.path.join(dir2, "decoded_tensors.meta")

    tensors1 = extract_tensors(list1_path)
    tensors2 = extract_tensors(list2_path)

    if tensors1 is None or tensors2 is None:
        return

    print("Validating tensor lists...")
    if len(tensors1) != len(tensors2):
        print(f"Mismatch: Dir1 has {len(tensors1)} tensors, Dir2 has {len(tensors2)} tensors.")
    else:
        print(f"Both directories list {len(tensors1)} tensors.")

    errors = 0
    verified = 0

    print("Starting bit-by-bit comparison...")
    for idx, file1 in tensors1.items():
        file2 = tensors2.get(idx)

        if not file2:
            print(f"Error: Index {idx} missing in Dir2.")
            errors += 1
            continue

        if file1 != file2:
            print(f"Error: Name mismatch at index {idx} ({file1} vs {file2}).")
            errors += 1
            continue

        path1 = os.path.join(dir1, file1)
        path2 = os.path.join(dir2, file2)

        hash1 = calculate_md5(path1)
        hash2 = calculate_md5(path2)

        if not hash1:
            print(f"Missing file: {path1}")
            errors += 1
            continue
        if not hash2:
            print(f"Missing file: {path2}")
            errors += 1
            continue

        if hash1 == hash2:
            verified += 1
        else:
            print(f"INTEGRITY FAILURE: {file1} differs.")
            print(f"Dir1 MD5: {hash1}")
            print(f"Dir2 MD5: {hash2}")
            errors += 1

    print("\n--- Summary ---")
    if errors == 0:
        print(f"Success! {verified}/{len(tensors1)} files verified and match bit-by-bit.")
    else:
        print(f"Completed with {errors} errors.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compare tensor files between two directories.")
    parser.add_argument("dir1", help="Path to the first directory")
    parser.add_argument("dir2", help="Path to the second directory")

    args = parser.parse_args()

    compare_directories(args.dir1, args.dir2)