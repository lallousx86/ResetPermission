ResetPermission (c) Elias Bachaalany <lallousz-x86@yahoo.com>

Copyright
==========
This is a free utility.

If you want to host this package somewhere else please include this copyright file.

Check out the book: "Batchography: The Art of Batch Files Programming" -- http://lallouslab.net/2016/05/10/batchography/

History
=========
08/24/2013 - Initial version
08/30/2013 - Enclose the folder with quotes if it contains at least one space character
09/17/2013 - Added "Reset files permission" as a optional action
           - Added "Reset hidden and system files"
03/31/2014 - Fixed double backslash when folder is root
01/08/2014 - Added "Do not follow symbolic links" option
03/31/2015 - Allow editing of the generated command textbox
           - Added "More actions" to add Explorer shell context menu
11/03/2015 - Added /SKIPSL switch to takeown.exe

11/15/2015 - v1.1.3
           - Added HELP button to redirect to blog entry
           - Added warning when attempting to change permission of a root folder

02/13/2016 - v1.1.4
           - Minor code changes
           - Update the console window title when the commands execute

02/28/2016 - v1.1.5
           - Refactored code
           - Added the Advanced button
           - Added Backup/Restore permissions

06/14/2016 - v1.1.6
           - Made the dialog non-static
           - Refactored the code
           - bugfix: Add/Remove from the Explorer folder context menu did not work unless a folder was selected.

12/19/2016 - v1.1.7
           - Attempt to make ResetPermission AntiVirus false-positive free by using 
             local app data folder instead of temp and by not deleting the temp batch script

