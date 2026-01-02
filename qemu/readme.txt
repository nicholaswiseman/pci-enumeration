=== How to set up QEMU Windows10 VM (WIP)===

Obtain some Win10 iso
Add it to <qemuDir>/isos

Create a .qcow2 disk:
<qemuDir>/qemu-img.exe create -f qcow2 YourNameHere.qcow2 80G

In <qemuDir>:
>mkdir unattend
Add autounattend.xml to the new directory
TODO: is unattend necessary?

Run: 
<qemuDir>/first-boot.bat

When asked select "I don't have product key"
Select OS version (windows 10 home)

When asked what type of installation do you want?
Choose "Custom: Install Windows only (advanced)"

Select drive 0 and hit next

Wait for install...

When install hits some issue ("Why did my PC restart?")

Close VM and run:
<qemuDir>/boot.bat

Should auto log in as admin.

At desktop open admin cmd prompt:

>net user lab lab /add
>net localgroup administrators lab /add

reboot and log as lab

Setup testsigning:
bcdedit /set testsigning on
shutdown /r /t 0

Then copy your clean image somewhere safe, just in case.


