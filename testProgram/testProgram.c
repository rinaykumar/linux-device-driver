/**************************************************************
* Class:  CSC-415-01 Fall 2020
* Name: Rinay Kumar
* Student ID: 913859133
* GitHub ID: rinaykumar
* Project: Assignment 6 - Device Driver
*
* File: testProgram.c
*
* Description: This program utitilzes the Linux Kernel Module
* kernelMod to get the nth number in the Fibonacci Sequence 
*
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define BUFFER_LENGTH 256
#define RD_VALUE _IOR('a', 'b', int)
#define WR_VALUE _IOW('a', 'a', int)

static char receive[BUFFER_LENGTH];

// Function to determine suffix of number
char *suffix(int n)
{
   switch (n % 100)
   {
   case 11:
   case 12:
   case 13:
      return "th";
   default:
      switch (n % 10)
      {
      case 1:
         return "st";
      case 2:
         return "nd";
      case 3:
         return "rd";
      default:
         return "th";
      }
   }
}

int main()
{
   // Variables
   int ret, fd;
   char stringToSend[BUFFER_LENGTH];
   int value, number;

   printf("*****Start Program*****\n");

   // Open the device with read/write access
   fd = open("/dev/kernelmod", O_RDWR);
   if (fd < 0)
   {
      perror("ERROR: Failed to open the device.");
      return errno;
   }

   // Prompt for input string
   printf("Get the nth number in the Fibonacci Sequence\n");
   printf("Type in an integer greater than 0: ");
   scanf("%s", stringToSend);

   // Send the string to the kernel mod using write()
   ret = write(fd, stringToSend, strlen(stringToSend));
   if (ret < 0)
   {
      perror("ERROR: Failed to write to the device.");
      return errno;
   }

   // Read the response from the kernel mod using read()
   ret = read(fd, receive, BUFFER_LENGTH);
   if (ret < 0)
   {
      perror("ERROR: Failed to read from the device.");
      return errno;
   }

   // Relay the input
   printf("The value you entered: %s\n", receive);

   // Convert the string to an int
   number = atoi(receive);

   // Error checking on number
   if (number == 0)
   {
      printf("ERROR: Invalid entry of '%s'.\n", receive);
      printf("Try again with an integer greater than 0.\n");
      printf("*****End Program*****\n");
      close(fd);
      return 0;
   }

   // Send int to kernel mod using ioctl()
   ret = ioctl(fd, WR_VALUE, &number);
   if (ret < 0)
   {
      perror("ERROR: Failed to write to the device.");
      return errno;
   }

   // Read calculated Fibonacci value from kernel mod using ioctl()
   ret = ioctl(fd, RD_VALUE, &value);
   if (ret < 0)
   {
      perror("ERROR: Failed to read from the device.");
      return errno;
   }

   // Output result and cleanup
   printf("The %d%s number in the Fibonacci Sequence is: %d\n", number, suffix(number), value);
   printf("*****End Program*****\n");
   close(fd);

   return 0;
}
