#!/bin/sh

LOG='/dev/shm/query.log'

# this sources bashlib into your current environment
. /www/cgi-bin/bashlib

# The CGI response header
echo "Content-type: application/json"
echo ""

#echo '============================='
dbg=`param dbg`
cmd=`param cmd`
callback=`param callback`
wrap_type=`param wrap_type`

if [ -n "$callback" ]; then
        echo "$callback ("
fi

if [ -n "$cmd" ]; then
        # Replace "+" with space
        cmd=$(echo ${cmd} | tr "+" "\x20")
        if [ "$dbg" = 'y' ]; then
                echo "cmd:$cmd" > $LOG
        fi

        if [ -n "$wrap_type" ]; then

        case "$wrap_type" in
            'single_line')
                echo "{"
                echo -n "\"stdout\":\""
                # Interpret special characters: \ " and \n
                eval "$cmd" 2>&1 | sed 's/\\/\\u005C/g' | sed 's/"/\\u0022/g' | awk '{ printf "%s", $0 }'
                echo "\","
                echo "\"_result\":\"pass\""
                echo "}"
            ;;
            'multi_line')
                echo "{"
                echo -n "\"stdout\":\""
                # Interpret special characters: \ " and \n
                eval "$cmd" 2>&1 | sed 's/\\/\\u005C/g' | sed 's/"/\\u0022/g' | awk '{ printf "%s\\u000D", $0 }'
                echo "\","
                echo "\"_result\":\"pass\""
                echo "}"
            ;;
            drop | *)
                # For the case of no json return, we should redirect all output to /dev/null.
                eval "$cmd" 2>&1 > /dev/null
            ;;
        esac

        else
            # This case assumes the $cmd will return a valid json object
            eval "$cmd"
        fi
fi

if [ -n "$callback" ]; then
        echo ");"
fi

exit

echo "
{
     \"firstName\": \"John\",
     \"lastName\": \"Smith\",
     \"age\": 25,
     \"address\":
     {
         \"streetAddress\": \"21 2nd Street\",
         \"city\": \"New York\",
         \"state\": \"NY\",
         \"postalCode\": \"10021\"
     },
     \"phoneNumber\":
     [
         {
           \"type\": \"home\",
           \"number\": \"212 555-1234\"
         },
         {
           \"type\": \"fax\",
           \"number\": \"646 555-4567\"
         }
     ]
}
"
fi
exit
