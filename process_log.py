#!/usr/bin/env python3

import glob
import os

def read_event_list(file_path):
    event_mapping = {}
    with open(file_path, 'r') as file:
        for line_number, line in enumerate(file):
            event_name, event_id = line.strip().split("=")
            event_mapping[event_id] = event_name
    return event_mapping

def parse_log_file(file_path, event_mapping):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    # Split the sections
    section_split_index = lines.index('##\n')
    section_one = lines[:section_split_index]
    section_two = lines[section_split_index + 1:]

    # Parse variables in the first section
    variables = {}
    for line in section_one:
        if '=' in line:
            key, value = line.strip().split('=')
            variables[key] = int(value)

    # Ensure 'start_timestamp' is in variables
    if 'start_timestamp' not in variables:
        raise ValueError("start_timestamp not found in the first section")

    # Process each record in the second section
    processed_records = []
    for record in section_two:
        timestamp_str, event_id = record.strip().split(',')
        timestamp = int(timestamp_str)
        time_diff = timestamp - variables['start_timestamp']
        event_name = event_mapping.get(event_id, f"Unknown Event ID {event_id}")
        processed_records.append((time_diff, variables.get('rank', 'N/A'), event_name))

    return processed_records

def aggregate_results(directory_path, event_mapping):
    pattern = os.path.join(directory_path, 'tprobe_*.log')
    log_files = glob.glob(pattern)

    all_processed_records = []
    for file_path in log_files:
        processed_records = parse_log_file(file_path, event_mapping)
        all_processed_records.extend(processed_records)

    # Sort the records by time_diff (first element of the tuple)
    all_processed_records = sorted(all_processed_records, key=lambda x: x[0])

    return all_processed_records

def main():
    directory_path = '.'  # Replace with the path to your directory containing log files
    event_list_path = 'ev_list'  # Replace with the path to your event list file

    # Read the event list and create a mapping
    event_mapping = read_event_list(event_list_path)

    # Aggregate results from log files
    aggregated_records = aggregate_results(directory_path, event_mapping)

    print("Aggregated Records:")
    for record in aggregated_records:
        if record[1] == 1:
            print(f"Time: {record[0]}, \t\tRank: {record[1]}, Event Name: {record[2]}")
        else:
            print(f"Time: {record[0]}, Rank: {record[1]}, Event Name: {record[2]}")

if __name__ == '__main__':
    main()
