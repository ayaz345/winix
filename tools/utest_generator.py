import sys
import re

def get_prototypes(files):
    prototypes = set([])
    for file in files:
        with open(file) as f:
            for line in f:
                match = re.search(r"^\s*void\s+test_(\w+)\s*\(\s*\)\s*{", line)
                if match:
                    proto = f"test_{match.group(1)}"
                    if proto in prototypes:
                        raise ValueError(f"{proto} is duplciated")
                    prototypes.add(proto)
    return prototypes

def generate(prototypes):
    print("#include <sys/fcntl.h>")
    print("#include <stdio.h>")
    print("#include \"unit_test.h\"")
    print()
    for proto in prototypes:
        print(f"void {proto}();")
    print()
    print("void run_all_tests(){")
    for proto in prototypes:
        print(f"    printf(\"%s\\n\", \"running {proto}\");")
        print(f"    reset_fs();")
        print(f"    {proto}();")
        print(f"    printf(\"%s\\n\\n\", \"passed: {proto}\");")
        print()
    print(f"    printf(\"%d tests passed\\n\", {len(prototypes)});")
    print("}")

def main():
    if len(sys.argv) < 2:
        exit(1)
    files = sys.argv[1:]
    prototypes = get_prototypes(files)
    generate(prototypes)

if __name__ == '__main__':
    main()