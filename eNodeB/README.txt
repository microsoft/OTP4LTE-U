*********************************
*** PERFORMANCE DEBUGGING
*********************************

To run eNodeB, make sure you run it with Administrator priviledges. 
It is also advisable to switch off paging: My Computer/System properties/Advanced/Performance/Advanced/Virtual Memory
It is also advisable to install and run Interrupt-Affinity Policy Tool (http://download.microsoft.com/download/9/2/0/9200a84d-6c21-4226-9922-57ef1dae939e/Interrupt_Affinity_Policy_Tool.msi, also accessible through https://msdn.microsoft.com/en-us/library/windows/hardware/dn550976(v=vs.85).aspx) and make sure all your hardware interrupts go to Core 0 (currently, this is a tedious manual process for each device, but we can script it later if needed). 

For some performance measurements of PCI see file PCIperf.txt

To debug performance of eNodeB, use ETW. The process is as follows:
- If not already installed, install xperf from: \\cam-01-srv\DFSRoot\users\rjblack\bin\install-xperf.cmd
- Find xperf, located in C:\Program Files\WPTInternals\xperf.exe
- Start xperf using: xperf.exe -on DiagEasy
- Run eNodeB
- Stop xperf using: xperf.exe -d trace.etl
- Stop eNodeB
- Visualize the trace: wpa trace.etl
- If you need to turn on the stack, see xperf -help stackwalk
- For more help type: start wpt.chm
- To visualize preemptions, choose Computation/CPU Usage (Precise)/Timeline by CPU graph.
  If some CPU is shown idle this means that there were no preemptions during that time so no process is registered.

Some documentation on xperf fields can be found on:
https://randomascii.wordpress.com/2012/05/11/the-lost-xperf-documentationcpu-scheduling/
Excerpts: 

- InSwitchTime (us) is the length of time that a thread is running,
  from the context switch where it starts running to the context
  switch where another thread starts running. This can be verified by
  taking SwitchInTime (s) on the selected row, adding NewInSwitchTime
  (us) (remembering to divide by 1,000,000 to convert to seconds), and
  noting that the result is the SwitchInTime (s) for the next row -
  the next context switch.



*********************************
*** CORE DUMP DEBUGGING
*********************************

To debug seg faults, use ProcDump tool (https://technet.microsoft.com/en-us/sysinternals/dd996900.aspx).
The problem is that something takes over exception so we don't get core dump/Watson/VS.
One can run the tool with 
  "c:\Program Files\Sysinternals\procdump.exe" -ma -e -x . mac.out <params>
or in cygwin with 
  /cygdrive/c/Program\ Files/Sysinternals/procdump.exe -ma -e -x . mac.out <params>
This will take over all the exceptions and dump the core. The core can then be openned using Open|File in VS
and used to get info about the dump (including mem values, stack, PC, etc).
Make sure that LTE-MAC is set as the start-up project
