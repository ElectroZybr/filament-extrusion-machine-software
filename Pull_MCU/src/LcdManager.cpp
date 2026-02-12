#include <LcdManager.h>


int LcdManager::get_indx_obj(int index) {
  byte counter = 1;
  for (int i = 0; i < menuSize; i++) {
    if (menu[i].parent == currentMenu){
      if (counter++ == index){
        return i;
      }      
    }
  }
  return 0;
}

String leftPadString(String input, int targetLength) {
    while (input.length() < targetLength) {
      input = ' ' + input;
    }
    return input;
}

String LcdManager::floatToConstLen(float input, uint8_t decimals) {
    char buffer[10];
    if (fabs(input) >= 10.0f) {
        decimals--;
    }
    dtostrf(abs(input), 0, decimals, buffer);
    return String(buffer);
}

void LcdManager::printValue(int index, float value) {
    char buffer[5];
    dtostrf(value, 0, 2, buffer);
    lcd.setCursor(13, index);
    lcd.print(leftPadString(buffer, 6));
}

void LcdManager::printValue(int index, int value) {
    char buffer[6];
    itoa(value, buffer, 10);
    lcd.setCursor(13, index);
    lcd.print(leftPadString(buffer, 6));
}

void LcdManager::printDecodeValue(int index, MenuItem item) {
    lcd.setCursor(13, index);
    lcd.print(leftPadString(item.list[*item.valueI], 6));
}

void LcdManager::printCursor() {
  if (editMode) {        
    lcd.setCursor(19, cursorPos);
    lcd.print("<");
  } else {
    lcd.setCursor(0, cursorPos);
    lcd.print(">");
  }
}

void LcdManager::clearCursor() {        
    lcd.setCursor(0, cursorPos);
    lcd.print(" ");
    lcd.setCursor(19, cursorPos);
    lcd.print(" ");
}

void LcdManager::printFolder(uint8_t start_pos)
{ 
    lcd.clear();
    printCursor();

    int i = 0;
    if (start_pos == 0) {
        lcd.setCursor(6, 0);
        lcd.print(menu[currentMenu].name);
        i = 1;
    }
    
    for (i; i <= ((menu[currentMenu].numChildren > lcdRows) ? lcdRows : menu[currentMenu].numChildren); i++) {
        MenuItem item = menu[get_indx_obj(start_pos+i)];
        lcd.setCursor(1, i);
        lcd.print(item.name);
        if (item.numChildren == 0) {
            if (item.valueF) printValue(i, *item.valueF);
            if (item.valueI) {
                if (item.list) {
                    printDecodeValue(i, item);
                }
                else printValue(i, *item.valueI);
            }
        }
    }
}

void LcdManager::update(bool forced_update) { 
    // struct LastValue {
    //     int valueI;
    //     float valueF;
    //     bool error;
    // };

    // static LastValue* last_values = nullptr;

    // if (!last_values) {
    //     last_values = new LastValue[mainSize];

    //     for (int i = 0; i < mainSize; i++) {
    //         last_values[i].valueI = -1;
    //         last_values[i].valueF = -1;
    //     }
    // }

    struct LastValue {
        // enum { INT, FLOAT, STRING } valueType;
        int intData;
        float floatData;
        String textData;
        bool error;
    };

    static LastValue* last_values = nullptr;

    if (!last_values) {
        last_values = new LastValue[mainSize];
    }


    for (int i = 0; i < mainSize; i++) {
        Main item = lcd_main[i];

        // if (item.valueI) {
        //     if (*item.valueI != last_values[i].valueI || forced_update) {
        //         lcd.setCursor(item.cols+((item.print_name) ? strlen(item.name) : 0), item.rows);
        //         if (item.decode_list) {
        //             lcd.print(item.decode_list[*item.valueI]);
        //         } else if (!item.error) {
        //             lcd.print(*item.valueI);   
        //         } else {
        //             if (*item.error) lcd.print(item.err_name); 
        //             else lcd.print(*item.valueI);
        //         }
        //         last_values[i].valueI = *item.valueI;
        //     }
        // } else if (item.valueF) {
        //     if (*item.valueF != last_values[i].valueI || forced_update) {
        //         lcd.setCursor(item.cols+((item.print_name) ? strlen(item.name) : 0), item.rows);
        //         if (!item.error) {
        //             lcd.print(floatToConstLen(*item.valueF, 3));   
        //         }
        //         else {
        //             if (*item.error) lcd.print(item.err_name); 
        //             else lcd.print(floatToConstLen(*item.valueF, 3));
        //         }
        //         // if (*item.valueF < 0) {
        //         //     lcd.setCursor(item.cols-1, item.rows);
        //         //     lcd.print("-");
        //         // }
        //         last_values[i].valueF = *item.valueF;
        //     }
        // }


        switch (item.valueType) {
            case item.INT:
                if (*item.intData != last_values[i].intData || forced_update) {
                    lcd.setCursor(item.cols+((item.print_name) ? strlen(item.name) : 0), item.rows);
                    if (item.decode_list) {
                        lcd.print(item.decode_list[*item.intData]);
                    } else if (!item.error) {
                        lcd.print(*item.intData);   
                    } else {
                        if (*item.error != item.err_rev) lcd.print(item.err_name); 
                        else lcd.print(*item.intData);
                    }
                    last_values[i].intData = *item.intData;
                }
                break;
            case item.FLOAT:
                if (*item.floatData != last_values[i].floatData || forced_update) {
                    lcd.setCursor(item.cols+((item.print_name) ? strlen(item.name) : 0), item.rows);
                    if (!item.error) {
                        lcd.print(floatToConstLen(*item.floatData, 3));   
                    }
                    else {
                        if (*item.error != item.err_rev) lcd.print(item.err_name); 
                        else lcd.print(floatToConstLen(*item.floatData, 3));
                    }
                    // last_values[i].floatData = *item.floatData;
                }
                break;
            case item.STRING:
                if (*item.textData != last_values[i].textData || forced_update) {
                    
                    lcd.setCursor(item.cols+((item.print_name) ? strlen(item.name) : 0), item.rows);
                    if (!item.error) {
                        lcd.print(*item.textData);
                    }
                    else {
                        if (*item.error != item.err_rev) lcd.print(item.err_name); 
                        else lcd.print(*item.textData);
                    }
                    last_values[i].textData = *item.textData;
                }
                break;
        }
    }
}

void LcdManager::printMain() {
    lcd.clear();
    for (int i = 0; i < mainSize; i++) {
        Main item = lcd_main[i];
        if (item.print_name) {
            lcd.setCursor(item.cols, item.rows);
            lcd.print(item.name);
        }
    }
    update(true);
}

void LcdManager::Right() {
    if (editMode) {
        if (settingsFlag) {
            MenuItem item = menu[get_indx_obj(scrollPos+cursorPos)];
            if (item.valueF) {
                if (*item.valueF+item.edit_step > item.maxVal) *item.valueF = item.maxVal;
                else *item.valueF = *item.valueF + item.edit_step;
                printValue(cursorPos, *item.valueF);
            } else if (item.valueI) {
                if (*item.valueI+item.edit_step > item.maxVal) *item.valueI = item.maxVal;
                else *item.valueI = *item.valueI + item.edit_step;
                if (item.list) printDecodeValue(cursorPos, item);
                else printValue(cursorPos, *item.valueI);
            }
        }
    } else {
        if (settingsFlag) {
            if (cursorPos == lcdRows && scrollPos < menu[currentMenu].numChildren-lcdRows) {
                scrollPos++;
                printFolder(scrollPos);
            } else if (cursorPos < ((menu[currentMenu].numChildren > lcdRows) ? lcdRows : menu[currentMenu].numChildren)) {
                clearCursor();
                cursorPos++;
                printCursor();
            }
        }
    }
}

void LcdManager::Left() {
    if (editMode) {
        if (settingsFlag) {
            MenuItem item = menu[get_indx_obj(scrollPos+cursorPos)];
            if (item.valueF) {
                if (*item.valueF-item.edit_step < item.minVal) *item.valueF = item.minVal;
                else *item.valueF = *item.valueF - item.edit_step;
                printValue(cursorPos, *item.valueF);
            } else if (item.valueI) {
                if (*item.valueI-item.edit_step < item.minVal) *item.valueI = item.minVal;
                else *item.valueI = *item.valueI - item.edit_step;
                if (item.list) printDecodeValue(cursorPos, item);
                else printValue(cursorPos, *item.valueI);
            }
        }
    } else {
        if (settingsFlag) {
            if (cursorPos == 0 && scrollPos > 0) {
                scrollPos--;
                printFolder(scrollPos);
            } else if (cursorPos > 0) {
                clearCursor();
                cursorPos--;
                printCursor();
            }
        }
    }
}

void LcdManager::RightH() {
    if (editMode) {
        if (settingsFlag) {
            MenuItem item = menu[get_indx_obj(scrollPos+cursorPos)];
            if (*item.valueF+(item.edit_step * 0.1) > item.maxVal) *item.valueF = item.maxVal;
            else *item.valueF = *item.valueF + (item.edit_step * 0.1);
            printValue(cursorPos, *item.valueF);
        }
    }
}

void LcdManager::LeftH() {
    if (editMode) {
        if (settingsFlag) {
            MenuItem item = menu[get_indx_obj(scrollPos+cursorPos)];
            if (*item.valueF-(item.edit_step * 0.1) < item.minVal) *item.valueF = item.minVal;
            else *item.valueF = *item.valueF - (item.edit_step * 0.1);
            printValue(cursorPos, *item.valueF);
        }
    }
}

void LcdManager::press() {
    if (settingsFlag) {
        int current_obj_indx = get_indx_obj(scrollPos+cursorPos);
        if (menu[current_obj_indx].valueF != NULL || menu[current_obj_indx].valueI != NULL) {
            editMode = !editMode;
            clearCursor();
            printCursor();
            if (!editMode) {
                settingChanged = true;
            }
        } else {
            if (cursorPos == 0){
                if (menu[currentMenu].parent == -1) {
                    settingsFlag = !settingsFlag;
                    printMain();
                } else {
                    currentMenu = menu[currentMenu].parent;
                    cursorPos = 0;
                    scrollPos = 0;
                    printFolder();
                }
            } else {
                if (menu[current_obj_indx].firstChild != -1) {
                    currentMenu = current_obj_indx;
                    cursorPos = 0;
                    scrollPos = 0;
                    printFolder();      
                }
            }  
        }
    }
}

void LcdManager::Holded() {
    settingsFlag = !settingsFlag;
    if (settingsFlag) printFolder();
    else {
        printMain();
        currentMenu = 0;
        cursorPos = 0;
        scrollPos = 0;
        editMode = false;
    }
}

LcdManager::LcdManager(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows,
                       const Main* lcd_main, size_t main_count,
                       const MenuItem* menu, size_t menu_count) : 
    lcd(lcd_Addr, lcd_cols, lcd_rows),
    editMode(false),
    settingsFlag(false),
    currentMenu(0),
    cursorPos(0),
    menu(menu),
    lcd_main(lcd_main),
    mainSize(main_count),
    menuSize(menu_count),
    lcdRows(lcd_rows-1),
    lcdCols(lcd_cols) {}

void LcdManager::init() {
    lcd.init();
    lcd.backlight();
    printMain();
}

void LcdManager::tick() {
    if (!settingsFlag) update();
}
