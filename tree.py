import os
import sys

def print_tree(root_path, prefix=""):
    total_loc = 0

    try:
        entries = sorted(os.listdir(root_path))
    except PermissionError:
        print(prefix + "└── [Permission Denied]")
        return 0

    for i, name in enumerate(entries):
        path = os.path.join(root_path, name)
        is_last = i == len(entries) - 1

        connector = "└── " if is_last else "├── "
        print(prefix + connector + name)

        if os.path.isfile(path) and name.endswith(".cpp") or name.endswith(".hpp"):
            try:
                with open(path, "r", encoding="utf-8", errors="ignore") as f:
                    total_loc += sum(1 for _ in f)
            except Exception:
                pass

        if os.path.isdir(path):
            extension = "    " if is_last else "│   "
            total_loc += print_tree(path, prefix + extension)

    return total_loc


def main():
    if len(sys.argv) < 2:
        print("Usage: python tree.py <dir1> [dir2 ...]")
        sys.exit(1)

    grand_total = 0

    for directory in sys.argv[1:]:
        print(directory)
        count = print_tree(directory)
        grand_total += count
        print(f"\nTotal lines in C++ files: {count}\n")

    print(f"Grand total lines in all C++ files: {grand_total}")


if __name__ == "__main__":
    main()