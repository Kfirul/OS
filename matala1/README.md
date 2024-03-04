# Exercise 1 - Linux Processes, execv, Debugging, Libraries, Threads
The project were writen by:

* Kfir Zilbernagel
* Roy Ofer
* Tomer Gozlan
## Overview
This exercise covers various aspects of Linux programming, including working with processes, debugging, using libraries, and creating multi-threaded applications. The tasks involve debugging faulty programs, implementing a Pythagorean theorem calculator, creating a library for encoding/decoding messages, and building a simple file compression tool using multiple processes.

We have meticulously crafted a comprehensive PDF document that provides detailed explanations of the programs we developed for each task. This document includes clear instructions and running samples to facilitate your understanding of the implemented functionalities. 
[Download PDF Documentation](link_to_your_pdf_file.pdf)
 

## Task 1: Practice working with a debugger

This task is designed to enhance debugging skills in the C programming language. Participants are assigned specific bugs based on the check digit of their ID card. The bugs intentionally cause program crashes to simulate real-world scenarios:

- Bug A: Creates a program that crashes due to division by zero.
- Bug B: Generates a program that crashes due to illegal access to memory (memory not initialized).
- Bug C: Develops a program that crashes due to infinite recursion.
- For this submission, we will focus on solving Bugs B and C, as the check digit of our ID is 6 and 8,9.

## Task 2: Triangle Analysis with Math Library

In this task, the objective is to develop a program that takes three side lengths of a triangle as input from the user. The program should then determine whether these side lengths form a Pythagorean triangle. If so, the program calculates and displays the corresponding angles in radians.
We assumed that the user will give a,b,c according to a^2+b^2=c^2 in that order.

Execution
- Input: Execute the program and provide three side lengths when prompted.

- Validation: The program checks whether the input side lengths can form a Pythagorean triangle.

- Output: If the input is valid, the program prints the angles in radians. In case of invalid input, an "Error" message is displayed, and the program exits.

## Task 3: Writing a codec for encoding and decoding a given string
This section introduces a codec library for encoding and decoding polyalphabetic ciphers. You will find details on the implementation of:

- Codec creation
- Text encoding and decoding
- Memory deallocation

Before running the program use this:export LD_LIBRARY_PATH=.

## Task 4: Compression and Encryption using Processes

This task involves creating a compression and encryption tool similar to 'zip' using multiple processes. The primary functionalities include:

- tar(1): Archive a directory and its subdirectories, combining multiple files into a single large file.
- compress(1)/gzip(1): Compress a large file to reduce its size.
- gpg(1): Encrypt or decrypt a file for added security.
- Two commands, myzip and myunzip, are implemented, each taking a parameter via argv.

his program compresses a directory or file using tar, gzip, and gpg,
    and then encrypts it with a passphrase provided by the user.

    Usage: ./myzip <directory/file> <passphrase>

    After using this, delete the file the orignal file that you used, and write the following command:
    ./myunzip output.gpg <passphrase>
    Now you will retrive your original file 

