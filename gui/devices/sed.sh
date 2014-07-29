#!/bin/bash
cat "$1" | while read line
do
	orig=`echo "$line" | sed -ne '/ id=\"/ { s/\(<[^>]*\)\([ \t]\+id=\"[^\">]*\"\)\([^>]*>[^<>]*<\/.*>\)/\1\3/; s/\//\\\\\//g; s/\"/\\\"/g; s/\&/\\\&/g; p }'`
	if [ -n "$orig" ]
	then
		sed -i "s/$orig/`echo "$line" | sed -ne '/ id=\"/ {s/\//\\\\\//g; s/\"/\\\"/g; s/\&/\\\&/g; p }'`/g" "$2"
	fi
done

page='''\
		<page name="language_reload">\
			<object type="template" name="header" \/>\
\
			<object type="text" color="%text_color%">\
				<font resource="font" \/>\
				<placement x="%center_x%" y="%row1_header_y%" placement="5" \/>\
				<text id="language_select">Select Language<\/text>\
			<\/object>\
\
			<object type="checkbox">\
			<placement x="%col1_x%" y="%row1_text_y%" \/>\
				<font resource="font" color="%text_color%" \/>\
				<text id="set_lang_en">Set lang to En<\/text>\
				<data variable="tw_lang_name_en" \/>\
				<image checked="checkbox_true" unchecked="checkbox_false" \/>\
			<\/object>\
\
			<object type="checkbox">\ 
			<placement x="%col1_x%" y="%row2_text_y%" \/>\
				<font resource="font" color="%text_color%" \/>\
				<text id="set_lang_zh">Set lang to zh-CN<\/text>\
				<data variable="tw_lang_name_zh-CN" \/>\
				<image checked="checkbox_true" unchecked="checkbox_false" \/>\
			<\/object>\		
\
			<object type="button">\
				<placement x="%col_center_x%" y="%tz_set_y%" \/>\
				<font resource="font" color="%button_text_color%" \/>\
				<text id="lang_set">Set Language<\/text>\
				<image resource="main_button" \/>\
				<actions>\
					<action function="reload"><\/action>\
				<\/actions>\
			<\/object>\
\
			<object type="text" color="%text_color%">\
				<font resource="font" \/>\
				<placement x="%center_x%" y="%tz_current_y%" placement="5" \/>\
				<text id="language_current">Current Language: %tw_lang_name%<\/text>\
			<\/object>\
\
			<object type="action">\
				<touch key="home" \/>\
				<action function="page">main<\/action>\
			<\/object>\
			<object type="action">\
				<touch key="back" \/>\
				<action function="page">advanced<\/action>\
			<\/object>\
\
			<object type="template" name="footer" \/>\
		<\/page>'''

sed -i "s/<action function=\"reload\"><\/action>/<action function=\"page\">language_reload<\/action>/" "$2"
sed -i 's/^[ \t]*<\/pages>/'"$page"'\n\t<\/pages>/g' "$2"
