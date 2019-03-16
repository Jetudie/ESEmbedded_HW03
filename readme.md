HW03
===
This is the hw03 sample. Please follow the steps below.

# Build the Sample Program

1. Fork this repo to your own github account.

2. Clone the repo that you just forked.

3. Under the hw03 dir, use:

	* `make` to build.

	* `make clean` to clean the ouput files.

4. Extract `gnu-mcu-eclipse-qemu.zip` into hw03 dir. Under the path of hw03, start emulation with `make qemu`.

	See [Lecture 02 ─ Emulation with QEMU] for more details.

5. The sample is a minimal program for ARM Cortex-M4 devices, which enters `while(1);` after reset. Use gdb to get more details.

	See [ESEmbedded_HW02_Example] for knowing how to do the observation and how to use markdown for taking notes.

# Build Your Own Program

1. Edit main.c.

2. Make and run like the steps above.

3. Please avoid using hardware dependent C Standard library functions like `printf`, `malloc`, etc.

# HW03 Requirements

1. How do C functions pass and return parameters? Please describe the related standard used by the Application Binary Interface (ABI) for the ARM architecture.

2. Modify main.c to observe what you found.

3. You have to state how you designed the observation (code), and how you performed it.

	Just like how you did in HW02.

3. If there are any official data that define the rules, you can also use them as references.

4. Push your repo to your github. (Use .gitignore to exclude the output files like object files or executable files and the qemu bin folder)

[Lecture 02 ─ Emulation with QEMU]: http://www.nc.es.ncku.edu.tw/course/embedded/02/#Emulation-with-QEMU
[ESEmbedded_HW02_Example]: https://github.com/vwxyzjimmy/ESEmbedded_HW02_Example

--------------------

- [x] **If you volunteer to give the presentation next week, check this.**

--------------------

**★★★ Please take your note here ★★★**

## Document reading
+ From Procedure Call Standard for the ARM Architecture(AAPCS) in [Application Binary Interface (ABI) for the ARM Architecture]:
	+ §5.1.1 Core registers
		```
		The first four registers r0-r3 (a1-a4) are used to pass argument values into a subroutine and to return a result value from a function. They may also be used to hold intermediate values within a routine (but, in general, only between subroutine calls).
		```
	+ §5.4 Result Return 
		```
		The manner in which a result is returned from a function is determined by the type of that result.
		```
		for now, let's just focus on integers
		```
		A word-sized Fundamental Data Type (e.g., int, float) is returned in r0.
		```
	+ §5.5 Parameter Passing
		```
		The base standard provides for passing arguments in core registers (r0-r3) and on the stack. For subroutines that take a small number of parameters, only registers are used, greatly reducing the overhead of a call.
		```
		and composite type(see §4.3 Composite Types) is also brought up in the statement
		```
		When a Composite Type argument is assigned to core registers (either fully or partially), the behavior is as if the argument had been stored to memory at a word-aligned (4-byte) address and then loaded into consecutive registers using a suitable load-multiple instruction.
		```
[Application Binary Interface (ABI) for the ARM Architecture]: http://infocenter.arm.com/help/topic/com.arm.doc.subset.swdev.abi/index.html

## Modify main.c
+ `gcd` :Since the document states that the caller will use r0-r3 for subroutines that take a small number of parameters, we first design a gcd function which takes only two parameters
	```clike=
	int gcd(int a, int b)
	```
+ `add`: Let's try to pass more than four arguments that r0-r3 can't hold at one time
	```clike=
	int add(int a, int b, int c, int d, int e, int f)
	```
+ `sum`: Use an array to see what will happen in a composite-type situation
	```clike=
	int sum(int a[6])
	```
+ `reset_handler` calls the these functions in the above presented order
	
## Observation / Result
+ The `arm-none-eabi-objdump -D main.elf` in `makefile` shows the disassembly of executable file `main.elf`
	+ o1. `gcd`
		+ o1-1. In `reset_handler`, before calling `gcd`, it first loads 24 and 18 to `r0` and `r1`
			```=
			000000ac <reset_handler>:
			  ac:	b580      	push	{r7, lr}
			  ae:	b08c      	sub	sp, #48	; 0x30
			  b0:	af02      	add	r7, sp, #8
			  b2:	2318      	movs	r3, #24
			  b4:	627b      	str	r3, [r7, #36]	; 0x24  <- a = 24
			  b6:	2312      	movs	r3, #18
			  b8:	623b      	str	r3, [r7, #32]		<- b = 18
			  ba:	6a78      	ldr	r0, [r7, #36]	; 0x24  <- r0 = a = 24
			  bc:	6a39      	ldr	r1, [r7, #32]		<- r1 = b = 18
			  be:	f7ff ffa3 	bl	8 <gcd>
			  .....
			```
		+ o1-2. When we first arrives `gcd`, `gcd` stores `r0`, `r1`(currently 24, 18) to stack, and when the recursion ends it loads the result to `r0`
			```
			00000008 <gcd>:
			   8:	b580      	push	{r7, lr}
			   a:	b082      	sub	sp, #8
			   c:	af00      	add	r7, sp, #0
			   e:	6078      	str	r0, [r7, #4] 	<- store r0(24) to stack, a = 24
			  10:	6039      	str	r1, [r7, #0] 	<- store r1(18) to stack, b = 18
			  12:	683b      	ldr	r3, [r7, #0]
			  .....
			  18:	687b      	ldr	r3, [r7, #4]    <- recursion about to end, r3 = a = gcd(24, 18) = 6
			  1a:	e00c      	b.n	36 <gcd+0x2e>
			  .....
			  36:	4618      	mov	r0, r3		<- prepare to return a, r0 = a = 6
			  38:	3708      	adds	r7, #8
			  3a:	46bd      	mov	sp, r7
			  3c:	bd80      	pop	{r7, pc}
			  3e:	bf00      	nop
			```
	+ o2. `add`
		+ o2-1. In `reset_handler`, before calling `add`, it first loads (1,2,3,a) to `r0`-`r3` and pose b and c on the stack
			```=
			  c2:	61f8      	str	r0, [r7, #28]		<- store return value c = 6
			  c4:	6a3b      	ldr	r3, [r7, #32]	
			  c6:	9300      	str	r3, [sp, #0]		<- on stack = b = 18
			  c8:	69fb      	ldr	r3, [r7, #28]
			  ca:	9301      	str	r3, [sp, #4]		<- on stack = c = 6
			  cc:	2001      	movs	r0, #1			<- r0 = 1
			  ce:	2102      	movs	r1, #2			<- r1 = 2
			  d0:	2203      	movs	r2, #3			<- r2 = 3
			  d2:	6a7b      	ldr	r3, [r7, #36]	; 0x24  <- r3 = a = 24
			  d4:	f7ff ffb4 	bl	40 <add>
			  .....
			```
		+ o2-2. When we first arrives `add`, `add` stores `r0`, `r1`(currently 24, 18) to stack, and when the recursion ends it loads the result to `r0`
			```
			00000040 <add>:
			  40:	b480      	push	{r7}
			  42:	b085      	sub	sp, #20
			  44:	af00      	add	r7, sp, #0
			  46:	60f8      	str	r0, [r7, #12]	<- store r0(1) to stack
			  48:	60b9      	str	r1, [r7, #8]	<- store r1(2) to stack
			  4a:	607a      	str	r2, [r7, #4]	<- store r2(3) to stack
			  4c:	603b      	str	r3, [r7, #0]	<- store r3(24) to stack
			  4e:	68fa      	ldr	r2, [r7, #12]
			  .....
			  5c:	69bb      	ldr	r3, [r7, #24]	<- load 18 from stack
			  5e:	441a      	add	r2, r3
			  60:	69fb      	ldr	r3, [r7, #28]	<- load 6 from stack
			  62:	4413      	add	r3, r2
			  64:	4618      	mov	r0, r3		<- prepare to return, r0 = 1+2+3+24+18+6 = 54
			  66:	3714      	adds	r7, #20
			  68:	46bd      	mov	sp, r7
			  6a:	f85d 7b04 	ldr.w	r7, [sp], #4
			  6e:	4770      	bx	l
			```
	+ o3. `sum`
		+ o3-1. In `reset_handler`, before calling `sum`, it first loads (1,2,3,a) to `r0`-`r3` and pose b and c on the stack
			```=
			  d8:	61f8      	str	r0, [r7, #28]		<- store return value, c = 54
			  da:	2304      	movs	r3, #4
			  dc:	607b      	str	r3, [r7, #4]		<- on stack = 4
			  de:	2305      	movs	r3, #5
			  e0:	60bb      	str	r3, [r7, #8]		<- on stack = 5
			  e2:	2306      	movs	r3, #6
			  e4:	60fb      	str	r3, [r7, #12]		<- on stack = 6
			  e6:	6a7b      	ldr	r3, [r7, #36]	; 0x24
			  e8:	613b      	str	r3, [r7, #16]		<- on stack = a = 24
			  ea:	6a3b      	ldr	r3, [r7, #32]
			  ec:	617b      	str	r3, [r7, #20]		<- on stack = b = 18
			  ee:	69fb      	ldr	r3, [r7, #28]
			  f0:	61bb      	str	r3, [r7, #24]		<- on stack = c = 54
			  f2:	1d3b      	adds	r3, r7, #4
			  f4:	4618      	mov	r0, r3			<- pass the first address on stack, r0 = 4's address
			  f6:	f7ff ffbb 	bl	70 <sum>
			  .....
			```
		+ o3-2. When we first arrives `add`, `add` stores `r0`, `r1`(currently 24, 18) to stack, and when the recursion ends it loads the result to `r0`
			```=
			00000070 <sum>:
			  70:	b480      	push	{r7}
			  72:	b085      	sub	sp, #20
			  74:	af00      	add	r7, sp, #0
			  76:	6078      	str	r0, [r7, #4]	<- store r0 (address of 4) to stack
			  78:	2300      	movs	r3, #0
			  7a:	60fb      	str	r3, [r7, #12]	<- sum = 0
			  7c:	2300      	movs	r3, #0
			  7e:	60bb      	str	r3, [r7, #8]	<- i = 0
			  .....
			  86:	687a      	ldr	r2, [r7, #4]	<- r2 = address of 4
			  88:	4413      	add	r3, r2		<- i + address of 4
			  8a:	681b      	ldr	r3, [r3, #0]	<- r3 = a[i], directly access from stack
			  8c:	68fa      	ldr	r2, [r7, #12]	<- r2 = sum
			  8e:	4413      	add	r3, r2		<- sum + a[i]
			  90:	60fb      	str	r3, [r7, #12]	<- sum = sum + a[i]
			  92:	68bb      	ldr	r3, [r7, #8]
			  94:	3301      	adds	r3, #1
			  .....
			  9e:	68fb      	ldr	r3, [r7, #12]	<- prepare to return, r0 = sum = 111
			  a0:	4618      	mov	r0, r3
			  a2:	3714      	adds	r7, #20
			  a4:	46bd      	mov	sp, r7
			  a6:	f85d 7b04 	ldr.w	r7, [sp], #4
			  aa:	4770      	bx	lr
			```
+ The behavior is just the same as those stated in the document!
