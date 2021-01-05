
#pragma once

#ifndef __HTTP_PAGE_TPL_H__
#define __HTTP_PAGE_TPL_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_attr.h"

extern const char *html_header_redirect ICACHE_RODATA_ATTR;
extern const char *html_error ICACHE_RODATA_ATTR;

extern const char *html_page_start1 ICACHE_RODATA_ATTR; 
extern const char *html_page_start2 ICACHE_RODATA_ATTR; 

extern const char *html_page_end1 ICACHE_RODATA_ATTR;
extern const char *html_page_end2 ICACHE_RODATA_ATTR;
extern const char *html_page_end3 ICACHE_RODATA_ATTR;

extern const char *html_block_data_end ICACHE_RODATA_ATTR;  

extern const char *html_page_content_start ICACHE_RODATA_ATTR;
// extern const char *html_page_content_end ICACHE_RODATA_ATTR;
#define html_page_content_end html_block_data_end

extern const char *html_page_top_header1 ICACHE_RODATA_ATTR;
extern const char *html_page_top_header2 ICACHE_RODATA_ATTR;
extern const char *html_page_top_header3 ICACHE_RODATA_ATTR;
extern const char *html_page_top_header4 ICACHE_RODATA_ATTR;

extern const char *html_page_menu_item1 ICACHE_RODATA_ATTR;
extern const char *html_page_menu_item2 ICACHE_RODATA_ATTR;
extern const char *html_page_menu_item3 ICACHE_RODATA_ATTR;

extern const char *html_page_devinfo1 ICACHE_RODATA_ATTR;
extern const char *html_page_devinfo2 ICACHE_RODATA_ATTR;
//extern const char *html_page_devinfo3 ICACHE_RODATA_ATTR;

extern const char *html_page_reboot_button_block ICACHE_RODATA_ATTR;


// ====== data block  ======================================================================
extern const char *html_block_data_header_start ICACHE_RODATA_ATTR;   
extern const char *html_block_data_no_header_start ICACHE_RODATA_ATTR;   

// ====== forms  ======================================================================
extern const char *html_block_data_form_start ICACHE_RODATA_ATTR;  
extern const char *html_block_data_div_lf3 ICACHE_RODATA_ATTR;  

extern const char *html_block_data_form_submit ICACHE_RODATA_ATTR;
//extern const char *html_block_data_form_submit_set ICACHE_RODATA_ATTR;
//extern const char *html_block_data_form_submit_del ICACHE_RODATA_ATTR;

extern const char *html_block_data_form_end ICACHE_RODATA_ATTR;

extern const char *html_block_data_form_item_label ICACHE_RODATA_ATTR;
//extern const char *html_block_data_form_item_label_w65_label ICACHE_RODATA_ATTR;
extern const char *html_block_data_form_item_label_label ICACHE_RODATA_ATTR;
extern const char *html_block_data_form_item_label_edit ICACHE_RODATA_ATTR;
extern const char *html_block_data_form_item_label_edit_hex ICACHE_RODATA_ATTR;
extern const char *html_block_data_form_item_checkbox  ICACHE_RODATA_ATTR;
extern const char *html_block_data_form_item_radio  ICACHE_RODATA_ATTR;

// ======================= button ============================
extern const char *html_button ICACHE_RODATA_ATTR;


extern const char *html_selected ICACHE_RODATA_ATTR;
extern const char *html_select_end ICACHE_RODATA_ATTR;
extern const char *html_select_item ICACHE_RODATA_ATTR;

extern const char *html_checkbox_checked ICACHE_RODATA_ATTR;
#endif 