"""
Profile the current state of the code.

"""

import os
import sys
import subprocess

exe_location = os.path.join(os.path.dirname(sys.argv[0]),
                            "../solve_the_spire/x64/Release/"
                            "solve_the_spire.exe")
exe_location = os.path.abspath(exe_location)
assert os.path.isfile(exe_location)

arguments = ["--character=ironclad",
             "--fight=gremlin_nob"]


def get_output(command):
    """Run a command and return the stdout output."""
    return subprocess.check_output(command, shell=True).decode('utf-8')


command = exe_location + ' ' + ' '.join(arguments)
text = get_output(command)
print(text)
text = [x for x in text.split("\n") if x.startswith("PROFILE:")]
text = text[0].split(": ", 1)[1]
print(text)

open("profile.txt", "a").write(text)
