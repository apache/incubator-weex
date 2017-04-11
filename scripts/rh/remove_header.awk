# This script removes ANSI C file header comments
BEGIN { start = 0; incomment = 0;}

{
    if (start == 0)
    {
        if (incomment == 0)
        {
            # C++ single line comment
            if (/^\/\//)
            {
                # print "single line"
            }
            # single line comment
            else if (/^\/\*[^\/]*\*\/$/)
            {
                # print "single line"
            }
            # multi line comment
            else if (/^\/\*/)
            {
                #print "start comment"
                # start multi line comment
                incomment = 1
            }
            else
            {
                # first line that is not a comment, start normal output
                print $0;
                start = 1;
            }
        }
        else
        {
            # search for comment end
            if (/\*\//)
            {
                #print "end comment"
                incomment = 0;
            }
        }
    }
    else
    {
        # print the complete line for the rest of the file
        print $0
    }
}
