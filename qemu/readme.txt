=== How to set up QEMU Windows10 VM (WIP)===

Obtain some Win10 iso
Add it to <qemuDir>/isos

Create a .qcow2 disk:
<qemuDir>/qemu-img.exe create -f qcow2 YourNameHere.qcow2 80G

Run: 
<qemuDir>/first-boot.bat

When asked select "I don't have product key"
Select OS version (windows 10 home)

When asked what type of installation do you want?
Choose "Custom: Install Windows only (advanced)"

Select drive 0 and hit next

Wait for install...

When install hits some issue ("Why did my PC restart?")

Ctrl + Shift + F3
System will reboot

Should auto log in as admin.

At desktop open admin cmd prompt:

>net user test test /add
>net localgroup administrators test /add

Close VM and run:
<qemuDir>/boot.bat

log in as test

Setup testsigning:
bcdedit /set testsigning on
shutdown /r /t 0

Then copy your clean image somewhere safe, just in case.


