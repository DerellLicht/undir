//**********************************************************
//  This file is NOT compiled into the code,
//  it is only used for LINT exceptions.
//**********************************************************

//lint -e18    Symbol (various) redeclared (origin) conflicts 
//             with line 26, file C:\VC98\INCLUDE\msxml.h, module Qualify.cpp

//lint -e38  Offset of symbol inconsistent (conflicts with line 6268,
//             file C:\VC98\INCLUDE\objidl.h, module CMD_LINE.CPP)

//lint -esym(1055, strlen)

//lint -esym(526, strlen, localtime)

//lint -e114  Inconsistent structure declaration for tag '_userSTGMEDIUM'

//lint -e525   Negative indentation from line ...
//lint -e534   Ignoring return value of function
//lint -e539   Did not expect positive indentation from line ...
//lint -e641   Converting enum to int
//lint -e713   Loss of precision (unsigned to signed) (assignment)
//lint -e716   while(1) ... 
//lint -e725   Expected positive indentation from line ... 
//lint -e731   Boolean argument to equal/not equal
//lint -e732   Loss of sign (initialization) (size N to size K)
//lint -e734   Loss of precision (assignment) (N bits to K bits)
//lint -e737   Loss of sign in promotion from int to unsigned int
//lint -e751   local typedef 'uchar' ((location not available)) not referenced
//lint -e750   local macro not referenced
//lint -e754   local structure member not referenced
//lint -e768   global struct member not referenced
//lint -e769   global enumeration constant not referenced
//lint -e778   Constant expression evaluates to 0 in operation '*'

// this warning probably *is* meaningful... 
// I just don't understand it!!
//lint -e1401  member not initialized by constructor

//lint -e1712  default constructor not defined for class
//lint -e1732  new in constructor for class which has no assignment operator
//lint -e1733  new in constructor for class which has no copy constructor

//lint -esym(759,hStdIn) -esym(765,hStdIn)
//lint -esym(759,drandom) -esym(765,drandom) -esym(714,drandom) 
//lint -esym(759,dclreos) -esym(765,dclreos) -esym(714,dclreos) 
//lint -esym(759,ngotoxy) -esym(765,ngotoxy)
//lint -esym(759,clear_visible_rows) -esym(765,clear_visible_rows) -esym(714,clear_visible_rows) 
//lint -esym(759,control_handler) -esym(765, control_handler)
//lint -esym(759, perr) -esym(765, perr) 
//lint -esym(759,hide_cursor) -esym(765,hide_cursor) -esym(714,hide_cursor) 
//lint -esym(759,dprints) -esym(765,dprints) -esym(714,dprints) 

//lint -esym(765, dputsi) 
//lint -esym(765, is_CRLF_present) 
//lint -esym(759,dprintc) -esym(765,dprintc) -esym(714,dprintc) 
//lint -esym(759,get_char) -esym(765,get_char) -esym(714,get_char) 
//lint -esym(715,lfn_off) ;
//lint -esym(759,set_text_attr) -esym(765,set_text_attr) -esym(714,set_text_attr)
//lint -esym(759,get_system_message) -esym(765,get_system_message) -esym(714,get_system_message)

