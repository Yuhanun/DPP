import glob

line_count = 0

allowed_extensions = [".hpp", ".cpp", ".c", ".h"]

for file in glob.iglob("*/**", recursive=True):
    if not (any([file.endswith(extension) for extension in allowed_extensions])):
        continue
    with open(file) as f:
        line_count += len(f.readlines())

print(line_count)