# LEVEL 0

```sh
level0@RainFall:~$ cat /home/user/level1/.pass
cat: /home/user/level1/.pass: Permission denied
```

- Let's check what we have:
```sh
level0@RainFall:~$ ls -l
total 732
-rwsr-x---+ 1 level1 users 747441 Mar  6  2016 level0
```
The **s** in `rws` indicates that the setuid bit is set, meaning the program will run with the permissions of the file's owner (level1), not the current user (level0).

> **+**: Indicates that there are additional access control lists (ACLs) set on the file, which can affect permissions.

- Some tests:
```sh
level0@RainFall:~$ ./level0
Segmentation fault (core dumped)
level0@RainFall:~$ ./level0 test
No !
level0@RainFall:~$ ./level0 123
No !
```

- Let's start by analyzing the disassembly of the main function of the program:

```sh
level0@RainFall:~$ gdb ./level0 
...
(gdb) disassemble main
```

```nasm
   0x08048ec0 <+0>:	push   %ebp
   0x08048ec1 <+1>:	mov    %esp,%ebp
   0x08048ec3 <+3>:	and    $0xfffffff0,%esp
   0x08048ec6 <+6>:	sub    $0x20,%esp
   0x08048ec9 <+9>:	mov    0xc(%ebp),%eax
   0x08048ecc <+12>:	add    $0x4,%eax
   0x08048ecf <+15>:	mov    (%eax),%eax
   0x08048ed1 <+17>:	mov    %eax,(%esp)
   0x08048ed4 <+20>:	call   0x8049710 <atoi>
   0x08048ed9 <+25>:	cmp    $0x1a7,%eax
   0x08048ede <+30>:	jne    0x8048f58 <main+152>
   0x08048ee0 <+32>:	movl   $0x80c5348,(%esp)
   0x08048ee7 <+39>:	call   0x8050bf0 <strdup>
   0x08048eec <+44>:	mov    %eax,0x10(%esp)
   0x08048ef0 <+48>:	movl   $0x0,0x14(%esp)
   0x08048ef8 <+56>:	call   0x8054680 <getegid>
   0x08048efd <+61>:	mov    %eax,0x1c(%esp)
   0x08048f01 <+65>:	call   0x8054670 <geteuid>
   0x08048f06 <+70>:	mov    %eax,0x18(%esp)
   0x08048f0a <+74>:	mov    0x1c(%esp),%eax
   0x08048f0e <+78>:	mov    %eax,0x8(%esp)
   0x08048f12 <+82>:	mov    0x1c(%esp),%eax
   0x08048f16 <+86>:	mov    %eax,0x4(%esp)
   0x08048f1a <+90>:	mov    0x1c(%esp),%eax
   0x08048f1e <+94>:	mov    %eax,(%esp)
   0x08048f21 <+97>:	call   0x8054700 <setresgid>
   0x08048f26 <+102>:	mov    0x18(%esp),%eax
   0x08048f2a <+106>:	mov    %eax,0x8(%esp)
   0x08048f2e <+110>:	mov    0x18(%esp),%eax
   0x08048f32 <+114>:	mov    %eax,0x4(%esp)
   0x08048f36 <+118>:	mov    0x18(%esp),%eax
   0x08048f3a <+122>:	mov    %eax,(%esp)
   0x08048f3d <+125>:	call   0x8054690 <setresuid>
   0x08048f42 <+130>:	lea    0x10(%esp),%eax
   0x08048f46 <+134>:	mov    %eax,0x4(%esp)
   0x08048f4a <+138>:	movl   $0x80c5348,(%esp)
   0x08048f51 <+145>:	call   0x8054640 <execv>
   0x08048f56 <+150>:	jmp    0x8048f80 <main+192>
   0x08048f58 <+152>:	mov    0x80ee170,%eax
   0x08048f5d <+157>:	mov    %eax,%edx
   0x08048f5f <+159>:	mov    $0x80c5350,%eax
   0x08048f64 <+164>:	mov    %edx,0xc(%esp)
   0x08048f68 <+168>:	movl   $0x5,0x8(%esp)
   0x08048f70 <+176>:	movl   $0x1,0x4(%esp)
   0x08048f78 <+184>:	mov    %eax,(%esp)
   0x08048f7b <+187>:	call   0x804a230 <fwrite>
   0x08048f80 <+192>:	mov    $0x0,%eax
   0x08048f85 <+197>:	leave  
   0x08048f86 <+198>:	ret    
```


We can see that the program retrieves the argument passed to it via the command line (from **0xc(%ebp)**) then adds 4 bytes to the argument pointer and dereferences it to fetch the value.

> Note
> In a function *(32-bit system)*, the first argument passed to it is typically found at **8(%ebp)**, the second argument at **0xc(%ebp)**, and so on. 

After that the program calls the atoi function to convert our argument into an integer then it compares the result with the value **0x1a7** (which is **423** in decimal). 
This means the program expects an argument of **423** to proceed.

- We can now try providing the program with the value **423** as an argument:
```sh
level0@RainFall:~$ ./level0 423
$ whoami
level1
$ 
```

As we can see, the program grants us a shell with the user permissions of level1 after successfully comparing the argument. This gives us access to files that the level1 user owns.

- Now We can read level1 password:
```sh
$ cat /home/user/level1/.pass
1fe8a524fa4bec01ca4ea2a869af2a02260d4a7d5fe7e7c24d8617e6dca12d3a
```