import os
import struct
import argparse

def pack_directory(source_dir, output_file):
    """
    Packs the contents of a directory into a single output file.

    Args:
        source_dir (str): Path to the source directory to pack.
        output_file (str): Path to the output file to write the packed data.
    """
    with open(output_file, 'wb') as out_file:
        for root, _, files in os.walk(source_dir):
            for file_name in files:
                file_path = os.path.join(root, file_name)
                relative_path = os.path.relpath(file_path, source_dir)

                # Write file path length and path
                relative_path_encoded = relative_path.encode('utf-8')
                out_file.write(struct.pack('I', len(relative_path_encoded)))
                out_file.write(relative_path_encoded)

                # Write file size and content
                with open(file_path, 'rb') as f:
                    file_data = f.read()
                    out_file.write(struct.pack('I', len(file_data)))
                    out_file.write(file_data)

    print(f"Packed directory into: {output_file}")

def main():
    parser = argparse.ArgumentParser(description="Pack the contents of a directory into a single file.")
    parser.add_argument("source_dir", help="Path to the source directory to pack.")
    parser.add_argument("output_file", help="Path to the output file to write the packed data.")
    args = parser.parse_args()

    pack_directory(args.source_dir, args.output_file)

if __name__ == "__main__":
    main()
