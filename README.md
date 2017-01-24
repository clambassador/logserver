# logserver
log file viewer with search filtering for fast navigation

The basic idea is to read a log file and support fast searching and navigation.

Commands
--------

To navigate:
. home/T top
. end/G bottom
. up/down/pageup/pagedown for line moving
. shift+home/shift+end for moving to start / end of current line

To search:
. / enter search mode. type the search phrase and hit enter

To delete last search:
. backspace

To mark:
. ! insert a line of hyphens
. # enter comment mode. type a comment and hit enter to add to text

Each search keyword is coloured and searches can be combined together.

When searching, the text on the screen is filtered to only the search words.
Hitting TAB switching between showing all lines and only matching ones.
Navigation works among the lines visible.


Features to Add
---------------

. formats for different log styles to have default colouring and filtering
. force linewidth with breaks
. show two files and synchronize timestamps when navigating
. change highlighting colours
. save files systematically
. with piped inputs, the reader thread should timeout while reading to check if
the program is quitting.
