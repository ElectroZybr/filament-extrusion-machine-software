// #include <LcdManager.h>


// int LcdManager::get_indx_obj(int index) {
//   byte counter = 1;
//   for (int i = 0; i < menuSize; i++) {
//     if (menu[i].parent == currentMenu){
//       if (counter++ == index){
//         return i;
//       }      
//     }
//   }
//   return 0;
// }

// String leftPadString(String input, int targetLength) {
//     while (input.length() < targetLength) {
//       input = ' ' + input;
//     }
//     return input;
//   }

// String LcdManager::floatToConstLen(float input, uint8_t decimals) {
//     char buffer[10];
//     if (fabs(input) >= 10.0f) {
//         decimals--;
//     }
//     dtostrf(abs(input), 0, decimals, buffer);
//     return String(buffer);
// }

// void LcdManager::printValue(int index, float value) {
//   char buffer[5];
//   dtostrf(value, 0, 2, buffer);
//   lcd.setCursor(13, index);
//   lcd.print(leftPadString(buffer, 6));
// }

// void LcdManager::printCursor() {
//   if (editMode) {        
//     lcd.setCursor(19, cursorPos);
//     lcd.print("<");
//   } else {
//     lcd.setCursor(0, cursorPos);
//     lcd.print(">");
//   }
// }

// void LcdManager::clearCursor() {        
//     lcd.setCursor(0, cursorPos);
//     lcd.print(" ");
//     lcd.setCursor(19, cursorPos);
//     lcd.print(" ");
// }

// void LcdManager::printFolder()
// { 
//   lcd.clear();
//   printCursor();
//   lcd.setCursor(6, 0);
//   lcd.print(menu[currentMenu].name);
//   for (int i = 1; i <= menu[currentMenu].numChildren; i++) {
//     MenuItem item = menu[get_indx_obj(i)];
//     lcd.setCursor(1, i);
//     lcd.print(item.name);
//     if (item.numChildren == 0) {
//         printValue(i, *item.value);
//     }
//   }
// }

// void LcdManager::update(bool forced_update) { 
//     struct LastValue {
//         int valueI;
//         float valueF;
//         bool error;
//     };

//     static LastValue* last_values = nullptr;

//     if (!last_values) {
//         last_values = new LastValue[mainSize];

//         for (int i = 0; i < mainSize; i++) {
//             last_values[i].valueI = -1;
//             last_values[i].valueF = -1;
//         }
//     }

//     for (int i = 0; i < mainSize; i++) {
//         Main item = lcd_main[i];

//         if (item.valueI) {
//             if (*item.valueI != last_values[i].valueI || forced_update) {
//                 lcd.setCursor(item.cols+((item.print_name) ? strlen(item.name) : 0), item.rows);
//                 if (item.decode_list) {
//                     lcd.print(item.decode_list[*item.valueI]);
//                 } else if (!item.error) {
//                     lcd.print(*item.valueI);   
//                 } else {
//                     if (*item.error) lcd.print(item.err_name); 
//                     else lcd.print(*item.valueI);
//                 }
//                 last_values[i].valueI = *item.valueI;
//             }
//         } else if (item.valueF) {
//             if (*item.valueF != last_values[i].valueI || forced_update) {
//                 lcd.setCursor(item.cols+((item.print_name) ? strlen(item.name) : 0), item.rows);
//                 if (!item.error) {
//                     lcd.print(floatToConstLen(*item.valueF, 3));   
//                 }
//                 else {
//                     if (*item.error) lcd.print(item.err_name); 
//                     else lcd.print(floatToConstLen(*item.valueF, 3));
//                 }
//                 // if (*item.valueF < 0) {
//                 //     lcd.setCursor(item.cols-1, item.rows);
//                 //     lcd.print("-");
//                 // }
//                 last_values[i].valueF = *item.valueF;
//             }
//         }

//     }
// }

// void LcdManager::printMain() {
//     lcd.clear();
//     for (int i = 0; i < mainSize; i++) {
//         Main item = lcd_main[i];
//         if (item.print_name) {
//             lcd.setCursor(item.cols, item.rows);
//             lcd.print(item.name);
//         }
//     }
//     update(true);
// }

// void LcdManager::Right() {
//     if (editMode) {
//         if (settingsFlag) {
//             MenuItem item = menu[get_indx_obj(cursorPos)];
//             if (*item.value+item.edit_step > item.maxVal) *item.value = item.maxVal;
//             else *item.value = *item.value + item.edit_step;
//             printValue(cursorPos, *item.value);
//         }
//     } else {
//         if (settingsFlag && cursorPos < menu[currentMenu].numChildren) {
//             clearCursor();
//             cursorPos++;
//             printCursor();
//         }
//     }
// }

// void LcdManager::Left() {
//     if (editMode) {
//         if (settingsFlag) {
//             MenuItem item = menu[get_indx_obj(cursorPos)];
//             if (*item.value-item.edit_step < item.minVal) *item.value = item.minVal;
//             else *item.value = *item.value - item.edit_step;
//             printValue(cursorPos, *item.value);
//         }
//     } else {
//         if (settingsFlag && cursorPos > 0) {
//             clearCursor();
//             cursorPos--;
//             printCursor();
//         }
//     }
// }

// void LcdManager::RightH() {
//     if (editMode) {
//         if (settingsFlag) {
//             MenuItem item = menu[get_indx_obj(cursorPos)];
//             if (*item.value+(item.edit_step * 0.1) > item.maxVal) *item.value = item.maxVal;
//             else *item.value = *item.value + (item.edit_step * 0.1);
//             printValue(cursorPos, *item.value);
//         }
//     }
// }

// void LcdManager::LeftH() {
//     if (editMode) {
//         if (settingsFlag) {
//             MenuItem item = menu[get_indx_obj(cursorPos)];
//             if (*item.value-(item.edit_step * 0.1) < item.minVal) *item.value = item.minVal;
//             else *item.value = *item.value - (item.edit_step * 0.1);
//             printValue(cursorPos, *item.value);
//         }
//     }
// }

// void LcdManager::press() {
//     if (settingsFlag) {
//         int current_obj_indx = get_indx_obj(cursorPos);
//         if (menu[current_obj_indx].value != NULL) {
//             editMode = !editMode;
//             clearCursor();
//             printCursor();
//             if (!editMode) {
//                 settingChanged = true;
//             }
//         } else {
//             if (cursorPos == 0){
//                 if (menu[currentMenu].parent == -1) {
//                     settingsFlag = !settingsFlag;
//                     printMain();
//                 } else {
//                     currentMenu = menu[currentMenu].parent;
//                     cursorPos = 0;
//                     printFolder();
//                 }
//             } else {
//                 if (menu[current_obj_indx].firstChild != -1) {
//                     currentMenu = current_obj_indx;
//                     cursorPos = 0;
//                     printFolder();      
//                 }
//             }  
//         }
//     }
// }

// LcdManager::LcdManager(const Main* lcd_main, size_t main_count,
//                        const MenuItem* menu, size_t menu_count) : 
//     lcd(0x27, 20, 4),
//     editMode(false),
//     settingsFlag(false),
//     currentMenu(0),
//     cursorPos(0),
//     menu(menu),
//     lcd_main(lcd_main),
//     mainSize(main_count),
//     menuSize(menu_count) {}

// void LcdManager::init() {
//     lcd.init();
//     lcd.backlight();
//     printMain();
// }

// void LcdManager::tick() {
//     if (!settingsFlag) update();
// }
