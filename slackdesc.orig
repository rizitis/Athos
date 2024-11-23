#!/bin/sh
#
# Copyright (c) 2010 - 2022 Vasileios Porpodas
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
#    The above copyright notice and this permission notice shall be
#    included in all copies or substantial portions of the Software.
#
#    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#    OTHER DEALINGS IN THE SOFTWARE.
#
# This software is licensed under GPL 3 License. http://gplv3.fsf.org/

# ABOUT
# Slackdesc is a script that generates the slack-desc files for
# Slackware linux packages (http://www.slackware.org).
# The output is printed on the screen.

# Read from stdin until newline or LIMIT
# Usage: SIMPLEINPUT LIMIT
simpleinput() {
    local limit=$1
    read -a IN -n $limit
    $ECHO ${IN[@]}
}


# Read stdin until the special characters are typed in.
# or until charecter LIMIT is reached. Return the input string up
# to the LIMIT.
# Usage: GETINPUT LIMIT
getinput() {
    local limit=$1
    local i=2
    while [ "${ALL[i-2]}" != ";" ]||[ "${ALL[i-1]}" != ";" ] ;do 
        if [ $i -gt $limit ];then
            $ECHO "Warning: Character limit of $limit exceeded." 1>&2
            $ECHO -e $str
            exit -1
        fi
        read -a IN -n 1
        #convert newlines into spaces
        if [ "$IN" == "" ]; then IN=" "; fi
        ALL[i]=$IN
        str+=$IN
        i=$(($i+1))
    done
    local max=$(($i - 4))
    local str=`$ECHO $str|head -c $max`
    $ECHO -e $str
}

# Strip trailing whitespaces
strip() {
    echo "${@}" | xargs -0
}

# Format the given STRING in slack-desc format using PKGNAME prefix.
# Make sure that it takes up to LINES lines.
# Usage: FORMAT PKGNAME "STRING" LINES
format() {
    # LINES
    local max_lines=$3

    local lines=1
    local out=""
    local wordlist=( )
    local i=0

    # STRING
    for word in $2; do
        wordlist[$i]=$word
        i=$(($i + 1))
    done
    local max=${#wordlist[@]}

    # "PKGNAME: "
    local fmted_pname="$1: "
    local linebuffer="${fmted_pname}"
    i=0;
    while [ $i -le $max ]; do
        if [ $lines -le $max_lines ] ; then
            local linebuffer_chars=$(get_length "$linebuffer")
            local next_word=${wordlist[$i]}
            local next_word_chars=$(get_length ${next_word})
            # Test if adding the next word will overflow the screen width.
            local test_chars=$(($linebuffer_chars + $next_word_chars + 1)) 
            if [ $test_chars -lt $screen_width ]; then
                linebuffer+="$next_word "
            else
                lines=$(($lines + 1))
                # Commit linbuffer to the output buffer
                out+="$(strip ${linebuffer})\n"
                # Start a new linebuffer
                linebuffer="${fmted_pname}"
                # Redo the last one.
                i=$(($i - 1))
            fi
        else
            if [ $i -lt $((max)) ]; then
                OVERFLOW="Y"
                break
            fi
        fi
        i=$(($i + 1))
    done
    if [ $lines -le $max_lines ]; then
        # Strip trailing whitespaces
        out+="$(strip ${linebuffer})\n"
    fi

    #This is a hack. all other checks are done in sanity_check()
    if [ "$OVERFLOW" == "Y" ]; then
      if [ ! "$DISABLED_CHECKS" == "N" ]; then
          # FORCE EXIT ON ERROR
          check_size "$description" $description_limit "FORCE"
      fi
    fi


    # line padding: add lines until we reach max_lines
    for ((i=lines; i < max_lines; i++ )); do
        out+="$(strip ${fmted_pname})\n"
    done
    $PRINTF "$out"
    if [ "$OUTFILE" != "" ]; then
        $ECHO -e $out >> $OUTFILE
    fi
}

# Print help message.
# Usage HELP_MESSAGE
usage() {
$ECHO ""
$ECHO -e "Usage: $PKGNAME \"<package name>\" \"<Short description>\" \"<Long description>\" \"<http://homepage>\" [ -n ]\n\
\n\
Example: $PKGNAME \"slackdesc\" \"Slackware slack-desc generator tool\" \"$PKGNAME is a tool that generates the slack-desc files for Slackware linux packages. It is now compliant with Slackbuilds.\" \"http://slack-desc.sourceforge.net\" \n\
\n\
Arguments description\n\
<package name>       Single word; The name of the package (application name)\n\
<short description>  A short description that fits one line\n\
<long description>   A description up to 7 lines long\n\
<http://homepage>    The project homepage. Should fit in 1 line\n\
-n                   Disable sanity checks\n\
"
}

# Get a string's length (count of characters)
# Usage: GET_LENGTH STRING
get_length() {
    string="$@"
    # don't use $ECHO because it inserts extra newline chars, destroys count
    local length=`$PRINTF "$string"|$WC -c`
    $ECHO $length
}

# Parse the command line arguments
# Usage PARSE_INPUT [ARG1] [ARG2] ...
parse_input(){
    if [ "$1" == "" ]||[ "$1" == "--help" ] || [ "$1" == "-help" ] || [ $# -eq 0 ]; then
        usage
        exit -1
    fi;

    DISABLE_CHECKS="N"
    if [ "$5" == "-n" ]; then
        DISABLE_CHECKS="Y"
    fi

    # This is an arbitrary limit to make sure that the args are passed correctly
    package_name_limit=20

    package_name="$1"
    summary="$2"
    description="$3"
    homepage="$4"

    screen_width=$((72 + $(get_length "${package_name}")))

    local name_width=$(get_length "${package_name}: ")
    homepage_limit=$(($screen_width - $name_width))
    local package_name_width=$(get_length "$package_name")
    summary_limit=$(($screen_width - $name_width - $package_name_width))
    description_limit=$(($(($screen_width - $name_width -1)) * $desc_lines))
}


# Check whether the STRING's size is less than or equal to MAX_LENGTH
# If FORCE_ERROR is "FORCE" then ignore the check and fail.
# Usage: CHECK_SIZE STRING MAX_LENGTH [FORCE_ERROR]
check_size() {
    local string="$1"
    local length=$2
    local FORCE_ERROR=$3
    local chars=`get_length "$string"`
    if [ $chars -ge ${length} ]||[ "$FORCE_ERROR" == "FORCE" ]; then
        $ECHO "ERROR! string: '${string}' too big!"
        usage
        exit -1
    fi
}

# Sanity checks about input's character lengths
# Usage: SANITY_CHECKS
sanity_checks() {
    if [ ! "DISABLE_CHECKS" == "Y" ]; then
        check_size "$package_name" $package_name_limit
        check_size "$package_name: $package_name $version" $(($screen_width))
        check_size "$summary" $summary_limit
        check_size "$package_name: $package_name $version $summary" $(($screen_width))
        check_size "$homepage" $homepage_limit
        check_size "$description" $description_limit
    fi
}

# Print the |----habndy-ruler----...--|
# Usage: HANDY_RULER
handy_ruler() {
    cat <<EOF
# HOW TO EDIT THIS FILE:
# The "handy ruler" below makes it easier to edit a package description.
# Line up the first '|' above the ':' following the base package name, and
# the '|' on the right side marks the last column you can put a character in.
# You must make exactly 11 lines for the formatting to be correct.  It's also
# customary to leave one space after the ':' except on otherwise blank lines.

EOF
    # $ECHO "     |-----handy-ruler------------------------------------------------------|"
    local space_cnt=$(get_length ${package_name})
    local cnt=0
    local gap=""
    while [ ${cnt} -lt ${space_cnt} ]; do
        gap="${gap} "
        cnt=$((${cnt} + 1))
    done
    local HANDY_RULER="${gap}|-----handy-ruler--"
    check_size "$HANDY_RULER" $screen_width
    local chars=$(get_length "$HANDY_RULER")
    while [ $chars -lt $screen_width ]; do
        HANDY_RULER+="-"
        chars=$(get_length "${HANDY_RULER}-")
    done
    HANDY_RULER+="|"
    $ECHO "$HANDY_RULER"
}

# Print the slack-desc output
# Usage: PRINT_OUTPUT
print_output() {
    handy_ruler
    sanity_checks
    format ${package_name} "$package_name ($summary)" $summ_lines 
    format ${package_name} "" $empty_line
    format ${package_name} "$description" $desc_lines
    format ${package_name} "$homepage" $homepage_lines
}


# Entry point
# Usage: MAIN [ARG1] [ARG2] ...
main() {
    parse_input "$1" "$2" "$3" "$4" "$5" "$6"
    print_output
}

# GENERAL CONFIGURATION
# paths to programs used
WC=/bin/wc
PRINTF=/bin/printf
ECHO=/bin/echo

PKGNAME="slackdesc"

summ_lines=1
empty_line=1
desc_lines=8
homepage_lines=1

main "$1" "$2" "$3" "$4" "$5" "$6"

