#!/bin/bash

# Process all .h files in the current directory (modify as needed for recursion)
for file in *.h; do
    # Skip if no .h files are found
    [ -f "$file" ] || continue

    # Generate the include guard name (convert filename to uppercase with underscores)
    guard_name=$(echo "${file^^}" | sed 's/\./_/g')

    # Check if the file already has an include guard (look for #ifndef)
    if grep -q "#ifndef ${guard_name}" "$file"; then
        echo "Skipping $file (already has include guards)"
        continue
    fi

    # Create a temporary file
    tmp_file=$(mktemp)

    # Write the include guard at the beginning
    echo "#ifndef ${guard_name}" > "$tmp_file"
    echo "#define ${guard_name}" >> "$tmp_file"
    echo "" >> "$tmp_file"

    # Append the original file content
    cat "$file" >> "$tmp_file"

    # Write the closing #endif at the end
    echo "" >> "$tmp_file"
    echo "#endif // ${guard_name}" >> "$tmp_file"

    # Overwrite the original file with the modified content
    mv "$tmp_file" "$file"

    echo "Added include guards to $file"
done

