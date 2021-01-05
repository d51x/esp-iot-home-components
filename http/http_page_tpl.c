#include "http_page_tpl.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


const char *html_header_redirect   = "<head><meta http-equiv='refresh' content='%d; URL=%s' /></head>";
const char *html_error   = "ERROR";





//char *menu_uri[MENU_ITEM_COUNT] = {"/", "/setup", "/tools", "/update", "/debug"};   // 10 char
//char *menu_names[MENU_ITEM_COUNT] = {"Main", "Setup", "Tools", "Update", "Debug"};  // 10 char

const char *html_block_data_end  = 
		"</div>";  

const char *html_page_start1 =    
  "<!DOCTYPE html>"
  "<html>"
    "<head>"
      "<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>"
      "<title>"; 

const char *html_page_start2 =    
  "</title>"
      "<meta name='viewport' content='width=480'>"
      "<meta name='mobile-web-app-capable' content='yes'><link rel='stylesheet' type='text/css' href='main.css'/>"
    "</head>"
    "<body>"
      "<div id='main'>"; 

const char *html_page_end1  = 
      "<script type='text/javascript' src='ajax.js'></script>"
      "<div id='clear'></div>"
        "<div id='footer' class='rnd'>"
          "<div id='time'>"
		  //"<span>"
		  "<b>";

const char *html_page_end2  = 
      "</b></div>"            // local time
          "<div id='fw'>"
		  //"<span>"
		  "<b>v.";
  
const char *html_page_end3  = 
      "</b>"
	  //"</span>"
	  "</div>"     // fw version
        "</div>"
      "</div>"
	  "</body>"
  "</html>";

const char *html_page_content_start  = "<div class='data'>";

//const char *html_page_content_end  = "</div>";

const char *html_page_top_header1  = 
  "<div id='header' class='bg'>"
    "<div id='host'>"
      "<h2>";

const char *html_page_top_header2  = 
 "</h2>"   // hostname
    "</div>"                                                              // 
    "<div id='head-right' class='dropdown'>"		                
      "<div id='rssi'>"
        "<h4>";

const char *html_page_top_header3  = 
 " dBm</h4>"
      "</div>"
	    "<div class='dropdown'>"
        "<button class='dropbtn'></button>"
	      "<div class='dropdown-content bg'>"
	        "<ul>";

const char *html_page_top_header4  = 
 "</ul>" // menu items
	      "</div>"
	    "</div>"
	  "</div>"
	"</div>"
	;

const char *html_page_menu_item1  =  "<li><a href='";
const char *html_page_menu_item2  =  "'>";
const char *html_page_menu_item3  =  "</a></li>";

const char *html_page_devinfo1  = 
  "<div id='dev-info' class='rnd brd-btm'>"
	  "<div id='memory'>"
	  //"<span>"
	  "<b>Free heap: </b> ";

const char *html_page_devinfo2  = 
  //"</span>"
  "</div>"  // free mem
    "<div id='uptime'>"
	//"<span>"
	"<b>Uptime: </b> ";

// const char *html_page_devinfo3  = 
//   //"</span>"
//   "</div>"     // uptime
//   "</div>";



const char *html_page_reboot_button_block  = 
	"<div class='group rnd'><div class='lf2'>" 
		"<button id='reboot' class='button off rht' onclick='reboot()'>Перезагрузить</button>" 
		"<div id='rbt'>Rebooting... Please, wait!</div>" 
		"</div>"
	"</div>";


// ====== data block  ======================================================================
const char *html_block_data_header_start  = 
		"<div class='group rnd'>"
				"<h4 class='brd-btm'>%s</h4>";  
				
const char *html_block_data_no_header_start  = 
		"<div class='group rnd'>";   



// ====== forms  ======================================================================
const char *html_block_data_form_start  = 
		"<form method='GET'>";  

const char *html_block_data_div_lf3  = 
		"<div class='lf3'>";  

const char *html_block_data_form_submit  = 

			//"<div class='rh2'><p><input type='hidden' name='st' value='%s'></p>"
			//"<div class='rh2'>"
			"<input type='hidden' name='st' value='%s'>"
			//"<p><input type='submit' value='Сохранить' class='button norm rht'></p>";
			"<input type='submit' value='Сохранить' class='button norm rht'>";

// const char *html_block_data_form_submit_set  = 

// 			//"<p><input type='hidden' name='st' value='%s'></p>"
// 			"<input type='hidden' name='st' value='%s'>"
// 			//"<p ><input type='submit' value='Сохранить' class='button norm'></p>";
// 			"<input type='submit' value='Сохранить' class='button norm'>";

// const char *html_block_data_form_submit_del  = 

// 			//"<p><input type='hidden' name='st' value='%s'></p>"
// 			"<input type='hidden' name='st' value='%s'>"
// 			//"<p><input type='submit' value='Удалить' class='button norm'></p>";
// 			"<input type='submit' value='Удалить' class='button norm'>";


const char *html_block_data_form_end  = 
		"</form>";

//const char *html_block_data_form_item_label_w65_label  = 
		//"<p><label class='lf' style='width:65%%'>%s: </label><label>%s</label></p>";
//		"<label class='lf' style='width:65%%'>%s: </label><label>%s</label>";

const char *html_block_data_form_item_label  = 
		"<label class='lf'>%s: </label>";


const char *html_block_data_form_item_label_label  = 
		//"<p><label class='lf'>%s: </label><label>%s</label></p>";
		"<label class='lf'>%s: </label><label>%s</label>";

const char *html_block_data_form_item_label_edit  = 
		//"<p><label class='lf'>%s: </label><input size='20' name='%s' class='edit rh' value='%s' /></p>";
		"<label class='lf'>%s: </label><input size='20' name='%s' class='edit rh' value='%s' />";

const char *html_block_data_form_item_label_edit_hex  = 
		//"<p><label class='lf'>%s: </label><input size='20' name='%s' class='edit rh' value='%s' /></p>";
		"<label class='lf'>%s: </label><input size='20' name='%s' class='edit rh' value='%s' />";

const char *html_block_data_form_item_edit_edit  = 
		//"<p><input size='20' name='%s' class='edit rh' value='%s' />/<input size='20' name='%s' class='edit rh' value='%s' /></p>";
		"<input size='20' name='%s' class='edit rh' value='%s' />/<input size='20' name='%s' class='edit rh' value='%s' />";


const char *html_block_data_form_item_checkbox  = 
		//"<p><label class='lf'>%s: </label><input type='checkbox' name='%s' class='edit rh' value='%d' %s /></p>";
		"<label class='lf'>%s: </label><input type='checkbox' name='%s' class='edit rh' value='%d' %s />";

const char *html_block_data_form_item_radio  = 
	"<input type='radio' name='%s' value='%s' %s />%s" ;

// ============ buttons =========================================
const char *html_button  =
																									 
		 "<button id='%s' class='button %s %s' "       //relay1/2...       lht/rht     // on или off - текущее состояние
											"data-class='button %s' "   // lht/rht
											"data-uri='%s' "   // %s = /uri?pin=%d&st=
											"data-val='%d' "                    // 0 или 1 - нужное состояние кнопки,которое будет передано в запрос для изменения
											"data-text='%s' "                    // текст для кнопки, который подставится после нажатия, парметр замены {0}
																					//id     id2    v   st
											"onclick='btnclick(this.id, this.id, %d, %d)'>"     // lcd - id, v: 0 - без подстановки результата, 1 - с подстановкой в конец, 2 - с подстановкой во внутрь вместо {0}
																																					// st: менять состояние кнопки 1, не менять состояние кнопки 0
											"%s"
			"</button>" ;



const char *html_selected = "selected=\"selected\" ";			
const char *html_select_end = "</select>";
const char *html_select_item = "<option value=\"%d\" %s>%s</option>";

const char *html_checkbox_checked = "checked";