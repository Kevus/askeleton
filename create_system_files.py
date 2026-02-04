import os
import json

DATA = "data"
TEMPLATE = "templates"
FRAMEWORKS = "frameworks"
ASKELETON_HOME = "ASKELETON_HOME"
FILE = "file"
FRAMEWORKS_DEFAULT = ["boost", "catch2", "gtest"]

askeleton_home = os.getenv(ASKELETON_HOME, os.getcwd())
config_path = os.path.join(askeleton_home, DATA, "configuration.json")

with open(config_path, "r") as f:
    config = json.load(f)

frameworks = config.get(FRAMEWORKS, FRAMEWORKS_DEFAULT)
files_list = []

template_files = config[FILE]["template"]
for key, value in template_files.items():
    if key == "cfg_tpl":
        files_list.append(os.path.join(askeleton_home, DATA, TEMPLATE, value))
    elif key == "method":
        for method_key, method_value in value.items():
            files_list.append(os.path.join(askeleton_home, DATA, TEMPLATE, method_value))
    elif isinstance(value, dict): 
        for framework in frameworks:
            for subkey, subvalue in value.items():
                files_list.append(os.path.join(askeleton_home, DATA, TEMPLATE, framework, subvalue))
    else:
        for framework in frameworks:
            files_list.append(os.path.join(askeleton_home, DATA, TEMPLATE, framework, value))

data_files = config[FILE][DATA]
for key, value in data_files.items():
    files_list.append(os.path.join(askeleton_home, value))

output_path = os.path.join(askeleton_home, DATA, "system_files.json")
with open(output_path, "w") as f:
    json.dump(files_list, f, indent=4)

print(f"File saved as {output_path}")
