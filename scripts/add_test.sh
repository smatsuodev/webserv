#!/bin/bash

# Get the absolute path
script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
project_root=$(dirname "$script_dir")

# Prompt user for test name (in snake_case)
read -r -p "Enter the name of the test (in snake_case): " test_name
if [ -z "$test_name" ]; then
  echo "Test name cannot be empty!"
  exit 1
fi

# Normalize the test name like "foo_test"
test_name=${test_name%.cpp} # Remove .cpp extension if present
if [[ ! $test_name == *_test ]]; then
  test_name="${test_name}_test" # Add _test suffix if NOT present
fi

# Set the path for the test file
test_file="$project_root/tests/${test_name}.cpp"
relative_test_file="tests/${test_name}.cpp"

# Check if the test file already exists
if [ -f "$test_file" ]; then
  echo "Test file already exists!"
  exit 0
fi

# Create the test file and copy the template
cp "$script_dir/template/test_template.cpp" "$test_file"

# Set the path for the CMakeLists.txt file
cmake_file="$project_root/tests/CMakeLists.txt"
relative_cmake_file="tests/CMakeLists.txt"

# Append to CMakeLists.txt
{
  echo
  echo "add_executable(${test_name} ${test_name}.cpp)"
  echo "gtest_discover_tests(${test_name})"
} >>"$cmake_file"

# Print the success message
echo "Test added successfully!"

# Display the list of changed files in a git-like format
echo ""
echo "Changes:"
echo "  A $relative_test_file"
echo "  M $relative_cmake_file"
