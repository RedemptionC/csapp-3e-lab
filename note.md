# Computer System:A Programmer's Perspective(CS:APP)

*一刷距今已经快两年了，这本书让我对cs的学习有了一个好的开始，在那之后，或者是刷新课，或者是研究生的繁琐事务，或者是实习，总之一直没有好好回顾CS:APP，这段时间感觉有点冲不动了，干脆来把CS:APP好好回顾一下。*

*当初最让我惊艳的，还是随书的lab，如果不是lab，很难想象我能坚持读完这么一本厚书，所以现在回顾的时候，还是以lab为主，再加上每个章节的重难点*

## official website

首先要关注的是本书的官网，*当初注意力主要在书和lab上，对这里没有足够的关注*，主要是 **student site**

### gdb

虽然现在很多课都会给一些gdb的使用说明，但是对gdb的使用，在csapp里是最多的，最开始用gdb估计也是在csapp里（*什么跨考生...*）

官网给出的gdb材料主要有两个：

* [beej's guide](http://beej.us/guide/bggdb/)
* [gdb on x86 cheat sheet](http://csapp.cs.cmu.edu/3e/docs/gdbnotes-x86-64.pdf)

比如我最近发现很好用的GDB TUI，在beej里面第一页就提到了，还是当初没做笔记的锅啊（*另外git也是，当时很多lab没有使用git记录，我现在都忘了csapp有没有强调使用git了*）

[这里还提到了DDD的教程，但是感觉整个都太老旧了](http://heather.cs.ucdavis.edu/~matloff/unix.html)

### 参考书

**The linux programming interface**

*我当时要是认真看了这个网站，就不会再去买APUE了。。*

### 剩下的主要就是lab相关的资料了

writeup什么的

在后面的章节里详细写

## chap1:A Tour of Computer Systems

**本章没有lab**

*并且因为本章讲的东西，太多太泛，对于我当时，还吸收不了那么多的内容，所以本章可能是吸收率最低的一章（一刷时）*

再看了下，发现这里确实是概括性质的，基本上后面每个章节都会对应有一小节，那么我也按照类似的结构吧

### Information Is Bits + Context

计算机中所有数据最终都是由bit即0/1表示，之所以能表示不同信息，是因为不同bit所在的context不同

### Programs Are Translated by Other Programs into Different Forms

![image-20210420152100654](https://raw.githubusercontent.com/RedemptionC/cloudimg/master/img/image-20210420152100654.png)

即，一个源代码，变成可执行程序，要经历如上几个阶段的转化（对c来说）

其中：

* 预处理：将包含的header插入到源文件中去
* 编译：源代码->汇编
* assembler：汇编->目标程序
* 链接：将使用的其他库函数（比如调用了printf,那么就是printf.o,即另一个目标文件）对应的目标文件，以某种方式与我们的目标文件

**↑ 这里涉及到预编译和separate compilation？**

### It Pays to Understand How Compilation Systems Work

为什么要学习编译系统的工作原理：

* 为了写出便于编译器优化的代码（第五章会介绍到）
* 理解link error（**至今不懂**）
* 避免安全漏洞（**这里书上举得例子是bufferoverflow，但是感觉和编译没什么关系？是和run-time overflow有关系？第三章会介绍到**）

### Processors Read and Interpret Instructions Stored in Memory

![image-20210420153004396](https://raw.githubusercontent.com/RedemptionC/cloudimg/master/img/image-20210420153004396.png)

首先给出了一个硬件的架构图：

* bus：在各个部件之间传递信息，传递单位是word，可能是32位或者64位（分别对应32和64位系统）
* IO设备：系统和外界通信的连接，比如鼠标键盘显示器硬盘
* memory：一般由DRAM组成，byte addressable，Volatile，所有要运行的程序/获取的数据 都需要先放入内存（写这段话的时候想到了DMA，其实direct memory access,怎么可能不通过内存呢,[DMA wiki](https://en.wikipedia.org/wiki/Direct_memory_access)）
* CPU:load/save/operate/jump是大概的操作流程，指令集规定了机器码的效果，而微体系架构则规定了CPU如何去执行这些指令（乱序，流水线等）

然后是hello程序的执行和这些硬件之间的关系：

![image-20210420155219218](https://raw.githubusercontent.com/RedemptionC/cloudimg/master/img/image-20210420155219218.png)

首先，要运行hello，在shell里输入文件路径，然后回车，该路径字符串就从io设备沿着上图中路径到达了内存中

上图中，数据到达内存，是经过了CPU（寄存器）的，实际上可以不经过，就是DMA：

![image-20210420155429635](https://raw.githubusercontent.com/RedemptionC/cloudimg/master/img/image-20210420155429635.png)

可执行文件加载到内存之后，cpu就可以执行其对应的机器命令了，对于本程序而言，就是将"hello"字符串（在内存中）打印在屏幕上：

### ![image-20210420155533988](https://raw.githubusercontent.com/RedemptionC/cloudimg/master/img/image-20210420155533988.png)

### Caches Matter

如上述几图，数据总是要在不同部件之间传来传去，而我们的目标只是在需要的地方使用它，因此需要尽量减少传输的时间

访问寄存器的内容很快，但是访问内存的速度要慢得多，因此在两者中间引入了cache，缓存经常要使用的内容：

![image-20210420160247472](https://raw.githubusercontent.com/RedemptionC/cloudimg/master/img/image-20210420160247472.png)

### Storage Devices Form a Hierarchy

![image-20210420160347184](https://raw.githubusercontent.com/RedemptionC/cloudimg/master/img/image-20210420160347184.png)

每一层都是作为下面一层的cache

### The Operating System Manages the Hardware

在hello的程序的执行过程中，用户程序（包括shell和hello）不会去直接操作硬件：比如从disk上读取hello文件，将字符串写入到显示设备上，和硬件直接打交道的是OS，用户程序使用的是OS提供的服务

OS提供两个功能：

* 防止硬件被滥用（*这很好理解，因为用户程序不能直接操作硬件了*）
* 向用户程序提供一组接口

OS主要使用了这几个abstraction：

* 进程：是对处理器，内存和io设备的抽象（**这个说法还没get到**）
* 虚拟内存 ： 是对内存和**磁盘文件**的抽象（**比如我们以为在内存中的，其实并不一定在？而是先从disk读到内存？**）
* 文件：是对IO设备的抽象

#### 进程

