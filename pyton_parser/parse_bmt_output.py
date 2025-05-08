#  SPDX-License-Identifier: MIT
# Copyright (c) 2025 Alejandro Avila Marcos

# Este archivo es parte de la librería Bare-Metal Test Framework (BMT).
#  BMT se distribuye bajo los términos de la Licencia MIT.
#  Puedes encontrar una copia de la licencia en el archivo LICENSE.txt
#  o en <https://opensource.org/licenses/MIT>.

import serial
import re
import time
import argparse

def parse_gtest_output_main_logic(port, baudrate, output_junit_file=None):
    print(f"Attempting to connect to {port} at {baudrate} baud...")
    try:
        ser = serial.Serial(port, baudrate, timeout=3) 
        print(f"Connected to {port}. Waiting for test output...")
    except serial.SerialException as e:
        print(f"Error opening serial port {port}: {e}")
        if output_junit_file: generate_empty_junit_xml(output_junit_file, f"Serial Port Error: {e}")
        return -1

    results = {
        "total_run": 0, "total_passed": 0, "total_failed": 0,
        "suites": {}
    }
    current_suite_for_failure = None
    current_test_for_failure = None
    in_test_run_phase = False
    explicit_end_token = "[BMT_DONE_ALL_TESTS]"
    re_running_tests = re.compile(r"\[==========\] Running (\d+) tests\.")
    re_run = re.compile(r"\[ RUN      \] (.*?)\.(.*)")
    re_ok = re.compile(r"\[       OK \] (.*?)\.(.*?) \((\d+|\d+\.\d+) ms\)")
    re_failed_line = re.compile(r"\[  FAILED  \] (.*?)\.(.*?) \((\d+|\d+\.\d+) ms\)")
    re_failure_location = re.compile(r"(.+?):(\d+): Failure")
    re_failure_assertion_type = re.compile(r"  (ASSERT_.+?|EXPECT_.+?|FAIL|ADD_FAILURE)\((.*?)\)")
    re_failure_message = re.compile(r"    Message: (.*)")
    max_idle_reads_after_start = 5
    idle_reads_count = 0

    try:
        while True:
            line_content = None
            try:
                line_bytes = ser.readline()
                if not line_bytes:
                    if not in_test_run_phase:
                        print("DEBUG: No data yet, waiting for tests to start...")
                        time.sleep(0.5)
                        continue
                    else:
                        idle_reads_count += 1
                        print(f"DEBUG: Idle read ({idle_reads_count}/{max_idle_reads_after_start}) after tests started.")
                        if idle_reads_count >= max_idle_reads_after_start:
                            print(f"Max idle reads ({max_idle_reads_after_start}) reached after tests started. Assuming end or stall.")
                            break
                        continue
                line_content = line_bytes.decode('utf-8', errors='replace').strip()
                idle_reads_count = 0
            except serial.SerialTimeoutException:
                print("DEBUG: SerialTimeoutException (should not happen with readline behavior).")
                if not in_test_run_phase: continue
                else:
                    idle_reads_count +=1
                    if idle_reads_count >= max_idle_reads_after_start: break
                    continue
            except Exception as e:
                print(f"Error reading from serial port: {e}")
                if output_junit_file: generate_empty_junit_xml(output_junit_file, f"Serial Read Error: {e}")
                return -2 
            if not line_content:
                if in_test_run_phase: print("DEBUG: Received an empty line after strip.")
                continue
            print(f"DUT: {line_content}")
            if explicit_end_token in line_content:
                print(f"Explicit end of tests token '{explicit_end_token}' received.")
                break
            match_running = re_running_tests.match(line_content)
            if match_running:
                in_test_run_phase = True
                current_suite_for_failure = None
                current_test_for_failure = None
                print("DEBUG: Detected test run start.")
                continue
            match_run = re_run.match(line_content)
            if match_run:
                current_suite_for_failure = match_run.group(1)
                current_test_for_failure = match_run.group(2)
                if current_suite_for_failure not in results["suites"]:
                    results["suites"][current_suite_for_failure] = {"passed": 0, "failed": 0, "tests": {}}
                results["suites"][current_suite_for_failure]["tests"][current_test_for_failure] = {
                    "name": current_test_for_failure, "classname": current_suite_for_failure, 
                    "status": "RUNNING", "duration_ms": 0, "failures": []}
                continue
            match_ok = re_ok.match(line_content)
            if match_ok:
                suite, test, duration_str = match_ok.groups()
                duration = float(duration_str)
                if suite in results["suites"] and test in results["suites"][suite]["tests"]:
                    results["suites"][suite]["tests"][test]["status"] = "OK"
                    results["suites"][suite]["tests"][test]["duration_ms"] = int(duration)
                    results["suites"][suite]["passed"] += 1
                    results["total_passed"] +=1
                else: print(f"Warning: [ OK ] for unknown test {suite}.{test}")
                results["total_run"] +=1
                current_suite_for_failure = None ; current_test_for_failure = None
                continue
            match_failed = re_failed_line.match(line_content)
            if match_failed:
                suite, test, duration_str = match_failed.groups()
                duration = float(duration_str)
                if suite in results["suites"] and test in results["suites"][suite]["tests"]:
                    results["suites"][suite]["tests"][test]["status"] = "FAILED"
                    results["suites"][suite]["tests"][test]["duration_ms"] = int(duration)
                    results["suites"][suite]["failed"] += 1
                    results["total_failed"] +=1
                else: print(f"Warning: [ FAILED ] for unknown test {suite}.{test}")
                results["total_run"] +=1
                continue
            if current_suite_for_failure and current_test_for_failure and \
               current_suite_for_failure in results["suites"] and \
               current_test_for_failure in results["suites"][current_suite_for_failure]["tests"]:
                test_obj = results["suites"][current_suite_for_failure]["tests"][current_test_for_failure]
                match_loc = re_failure_location.match(line_content)
                if match_loc:
                    file, lineno = match_loc.groups()
                    test_obj["failures"].append({"file": file, "line": lineno, "assertion": "", "expression": "", "message": ""})
                    continue
                if test_obj["failures"]:
                    last_failure_entry = test_obj["failures"][-1]
                    match_assert = re_failure_assertion_type.match(line_content)
                    if match_assert:
                        last_failure_entry["assertion"] = match_assert.group(1).strip()
                        last_failure_entry["expression"] = match_assert.group(2).strip()
                        continue
                    match_msg = re_failure_message.match(line_content)
                    if match_msg:
                        last_failure_entry["message"] = match_msg.group(1).strip()
                        continue
    except KeyboardInterrupt:
        print("\nInterrupted by user.")
        if output_junit_file: generate_empty_junit_xml(output_junit_file, "User Interruption")
        return -3
    except Exception as e:
        print(f"An unexpected error occurred during main parsing loop: {e}")
        if output_junit_file: generate_empty_junit_xml(output_junit_file, f"Unexpected Error: {e}")
        return -4
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print(f"Serial port {port} closed.")
    print("\n--- Test Run Summary (Console) ---")
    if not results["suites"] and results["total_run"] == 0 :
        print("No test results captured or no tests were run.")
        if output_junit_file: generate_empty_junit_xml(output_junit_file, "No tests run or captured")
        return 0 
    final_total_tests = results["total_run"]
    final_passed_tests = results["total_passed"]
    final_failed_tests = results["total_failed"]
    for suite_name, suite_data in results["suites"].items():
        print(f"\nSuite: {suite_name} (Passed: {suite_data['passed']}, Failed: {suite_data['failed']})")
        for test_name, test_data in suite_data["tests"].items():
            status_icon = "✅" if test_data["status"] == "OK" else ("❌" if test_data["status"] == "FAILED" else "❓")
            print(f"  {status_icon} {test_data['name']} ({test_data['duration_ms']} ms) - {test_data['status']}")
            for failure in test_data.get("failures", []):
                print(f"    └─ Fail @ {failure['file']}:{failure['line']}")
                if failure['assertion'] or failure['expression']:
                     print(f"       Assertion: {failure['assertion']}({failure['expression']})")
                if failure['message']:
                     print(f"       Message: {failure['message']}")
    print("\n------------------------------------")
    print(f"Total Tests Run: {final_total_tests}")
    print(f"Passed: {final_passed_tests}")
    print(f"Failed: {final_failed_tests}")
    print("------------------------------------")

    if output_junit_file:
        try:
            from junit_xml import TestSuite, TestCase 
            test_suites_list = []
            for suite_name, suite_data in results["suites"].items():
                test_cases = []
                for test_name_key, test_data_val in suite_data["tests"].items():
                    duration_sec = test_data_val['duration_ms'] / 1000.0
                    tc = TestCase(name=test_data_val['name'], classname=test_data_val['classname'], 
                                  elapsed_sec=duration_sec)
                    if test_data_val['status'] == "FAILED":
                        failure_message = ""
                        for fail_idx, f_detail in enumerate(test_data_val['failures']):
                            failure_type = f_detail.get('assertion', 'Failure')
                            failure_output = (
                                f"Location: {f_detail['file']}:{f_detail['line']}\n"
                                f"Expression: {f_detail.get('expression', 'N/A')}\n"
                                f"Message: {f_detail.get('message', 'N/A')}"
                            )
                            tc.add_failure_info(message=f"Failure {fail_idx+1}", 
                                                output=failure_output.strip(),
                                                failure_type=failure_type)
                    test_cases.append(tc)
                ts = TestSuite(name=suite_name, test_cases=test_cases)
                test_suites_list.append(ts)
            
            if test_suites_list:
                xml_string = TestSuite.to_xml_string(test_suites_list, prettyprint=True)
                with open(output_junit_file, 'w', encoding='utf-8') as f:
                    f.write(xml_string)
                print(f"JUnit XML report generated at {output_junit_file}")
            else:
                print("No test suites processed to generate JUnit XML.")
                generate_empty_junit_xml(output_junit_file, "No test suites were processed.")

        except ImportError: print("Warning: junit-xml library not found. Cannot generate JUnit XML report. (pip install junit-xml)")
        except Exception as e_junit: 
            print(f"Error generating JUnit XML report: {e_junit}")
            if output_junit_file:
                generate_empty_junit_xml(output_junit_file, f"JUnit Generation Error: {e_junit}")
    return final_failed_tests

def generate_empty_junit_xml(filename, message="No tests run or captured"):
    try:
        from junit_xml import TestSuite, TestCase
        tc = TestCase(name="FrameworkError", classname="BMT")
        tc.add_error_info(message=message, output=message, error_type="RunError")
        ts = TestSuite(name="BMTRun", test_cases=[tc]) 
        
        xml_string = TestSuite.to_xml_string([ts], prettyprint=True)
        with open(filename, 'w', encoding='utf-8') as f:
            f.write(xml_string)
        print(f"Empty/Error JUnit XML report generated at {filename}: {message}")
    except ImportError: pass
    except Exception as e: print(f"Error generating empty JUnit XML: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Parse BMT gtest-like output from serial and optionally generate JUnit XML.")
    parser.add_argument('--port', required=True, help="Serial port (e.g., COM3 or /dev/ttyUSB0)")
    parser.add_argument('--baud', type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument('--junit_xml', type=str, help="Filename to output JUnit XML report (e.g., test_results.xml)")
    args = parser.parse_args()
    num_failures = parse_gtest_output_main_logic(args.port, args.baud, args.junit_xml)
    if num_failures < 0:
        print(f"Script exited with an error code: {num_failures}")
        exit(abs(num_failures)) 
    elif num_failures > 0:
        print(f"Exiting with error code due to {num_failures} test failures.")
        exit(1)
    else:
        print("All tests passed or no failures detected. Exiting successfully.")
        exit(0)