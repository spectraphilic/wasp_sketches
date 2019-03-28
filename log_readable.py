from datetime import datetime
import sys


help_text = """
Makes it easier to read log files. Converts epoch time to human readable
format.

Example:

$ python log_readable.py LOG.TXT
2019-02-22 14:21:00.459000 DEBUG command ls
[...]
"""

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(help_text)
        sys.exit(1)

    filename = sys.argv[1]
    for line in open(filename):
        try:
            time, tail = line.split(' ', 1)
            time = datetime.utcfromtimestamp(float(time))
        except ValueError:
            pass
        else:
            line = f'{time} {tail}'

        print(line, end='')
