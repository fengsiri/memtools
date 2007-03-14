Address Space Monitor
=====================

Address Space Monitor is a simple utility to monitor a process' use of its
address space.

To begin monitoring a process, choose the "File|Attach ..." menu option and type
in the process id of the process to be monitored.  The process id can be
retrieved from the Windows Task Manager by choosing to display the "PID
(Process Identifier)" column.

The display is split into two halves.

The top half shows the address space of the process in a gauge format, from low
addresses (close to zero) on the left, rotating clockwise to high addresses (2GB
for 32-bit processes, 8TB for 64-bit processes) on the right.

Free address space is shown in green, reserved addresses in yellow and used
(committed) memory regions in red.

The bottom half of the display shows the largest free regions of address space
as coloured ellipses in descending order of size.  The full width of the display
represents a quarter of the total address space for the process (about 512MB for
32-bit processes, about 2TB for 64-bit processes) and large regions are shown as
half an ellipse covering the full width of the display even if they exceed half
the total address space.

The regions are coloured according to their upper address so that they can be
tracked as their lower parts are used up.  Low regions start from yellow, going
through magenta at half way, to cyan at the upper end of the address space.

The utility has most use for 32-bit processes as 64-bit process tend to be
dominated by a very large (only fractionally smaller that 8TB) region of free
address space from the 2GB boundary upwards.

Charles Bailey.
