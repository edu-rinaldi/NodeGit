import argparse
import os
# Note: If script is failing to launch executable, change ND_BLENDER_EXEC_PATH in run_single_test.py
from run_single_test import run_single_test

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", type=str, help="Path to the directory containing all the shading tests", default=None)
    parser.add_argument("-g", type=str, help="Path to the directory containing all the geometry tests", default=None)
    parsed = parser.parse_args()

    shading_tests_dir = parsed.s
    geoemetry_tests_dir = parsed.g
    
    # filters
    shading_filter = {".DS_Store"}
    geometry_filter = {".DS_Store"}
    
    shading_tests = filter(lambda dir: os.path.isdir(os.path.join(shading_tests_dir, dir)) and dir not in shading_filter, os.listdir(shading_tests_dir)) if shading_tests_dir is not None else []
    geometry_tests = filter(lambda dir: os.path.isdir(os.path.join(geoemetry_tests_dir, dir)) and dir not in geometry_filter, os.listdir(geoemetry_tests_dir)) if geoemetry_tests_dir is not None else []
    
    print("===\tTesting shading examples\t===")
    for preset_name in shading_tests:
        print(f"\tTesting preset: {preset_name}")
        run_single_test(os.path.join(shading_tests_dir, preset_name), preset_name, True)
    print()
    print("===\tTesting geometry examples\t===")
    for preset_name in geometry_tests:
        print(f"\tTesting preset: {preset_name}")
        run_single_test(os.path.join(geoemetry_tests_dir, preset_name), preset_name, False)

    print("Test finished")
if __name__ == "__main__":
    main()