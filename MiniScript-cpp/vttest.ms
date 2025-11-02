import "vt"

hex2 = function(val)
	digits = "0123456789ABCDEF"
	val = floor(val)
	if val < 0 then return "00"
	if val >= 255 then return "FF"
	return digits[val / 16] + digits[val % 16]
end function

print vt.enterApp
result = term_raw(true)        // enable raw+VT
if result isa string then print "Error: " + result

rowsCols = term_size
print vt.cursor.goto(1,1) + vt.normal + "Size: " + rowsCols

print vt.cursor.goto(3,1) + "Press keys; ESC twice to exit."
escSeen = false
while true
    s = term_read(64, 200)     // wait up to 200ms for any bytes
    if s != "" then
        // Show hex bytes for debugging
        hex = ""
        for ch in s
            hex += (" " + hex2(code(ch)))  // adjust to your hex format helper
        end for
        print vt.cursor.goto(5,1) + vt.clearLine + "Bytes:" + hex
        if s == char(27) then
            if escSeen then break else escSeen = true
        else
            escSeen = false
        end if
    end if
end while

term_raw(false)
print vt.leaveApp()
