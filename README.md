# Monitors
Monitor solution to producer consumer problem

There are M buffers ( of type A, B or C) , producer randomly chooses the buffer to produce. 
If buffer is epmty waits on condition full. As a result to every single buffer, 
there is a single product consumer assigned, that will consume the product to allow the producer to produce in a situation
described above ( he will send a singal to him). 

Some customers ( of type < 0 ) consume more than one product. There are of type -1 or -2. 
If their requirement is not met ( if in the buffers the number of needed elements is lower ),
they wait on condition multiplyWaiting - multiply stands for multiply products they want to consume.
They are signalled by producer if the last needed product appears.

When program launched, there are shown: number of Cumstomer -1 vists, Customer -2 visits, total number
of products consumed by A, B and C customers.

File monitor.h is a standard Hoare monitor. Since WUT has rights to this file I cannot publish it.


