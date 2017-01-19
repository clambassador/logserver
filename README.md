# logserver
log file viewer with search filtering for fast navigation

The basic idea is to read a log file and support fast searching and navigation.

Commands
--------

To navigate:
. T top
. G bottom
. up/down/pageup/pagedown/home/end as expected

To search:
. / enter search mode. type the search phrase and hit enter

To delete last search:
. backspace

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
. commands to insert lines into the log, such as markers indicating the point
before doing something that should produce output
. save files systematically
