# path-builder
Computer vision algorithm to detect left and right edges of a road and translate them to waypoints along a path.

Takes a bitmap image where 1=lane line pixel as input.

## usage
Edit the `main()` function in `main.cpp` to determine whether to run in test mode or not. Comment out the 
`#define _D_DEBUG` if you want to minimize debug information. The two available modes can be chosen by switching 
the return statement in the main function. Try `main_test()` and `main_udp()`. The udp version loads bitmaps 
images from port 1234 and sends out the processed path.

## other stuff
Please don't commit changes to .vcxproj. I don't know how to do this stuff properly.
