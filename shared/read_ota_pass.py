#!/usr/bin/env python3

Import ("env")

import os
import os.path as path

def read_callback(source, target, env):
    input = path.join(env.get("PROJECT_DIR"), "otapass.txt")

    if not path.exists(input) or not path.isfile(input):
        print(f"Error: {input} does not exist.")
        env.Exit(1)

    password = ""

    print("Reading OTA password.")

    with open(input) as f:
        password = f.readline()

    password = password.strip('\00')
    env.Append(UPLOADERFLAGS=["-a", password])

env.AddPreAction("upload", read_callback)
