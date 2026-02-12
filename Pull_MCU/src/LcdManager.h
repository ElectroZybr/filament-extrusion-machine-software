#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class LcdManager {
public: 
    typedef struct {
        const char* name;       // Название пункта меню
        int parent;             // Индекс родительского меню
        int firstChild;         // Индекс первого дочернего элемента
        int numChildren;        // Количество дочерних элементов
        float* valueF;          // Указатель на значение (если это параметр)
        int* valueI;
        int minVal;             // Минимальное значение параметра
        int maxVal;             // Максимальное значение параметра
        const char** list;      // Список значений для параметров-списков
        float edit_step;
      } MenuItem;

    struct Main {
        const char* name;       // Название пункта меню
        int cols;
        int rows;
        enum ValueType { INT, FLOAT, STRING } valueType;
        union {
            int* intData;
            float* floatData;
            String* textData;
        };
        const char** decode_list;      // Список значений для параметров-списков
        bool print_name;
        bool* error;
        const char* err_name;
        bool err_rev;
      };

    LcdManager(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows,
               const Main* lcd_main, size_t main_count, 
               const MenuItem* menu, size_t menu_count);  // Конструктор

    void init();
    void tick();

    void Right();
    void Left();
    void RightH();
    void LeftH();
    void press();
    void Holded();

    bool settingChanged = false;
    const char* status_decode;

    bool settingsFlag;
    
private:
    void printMain();
    void update(bool forced = false);
    void printFolder(uint8_t start_pos = 0);
    void printCursor();
    void printValue(int index, float value);
    void printValue(int index, int value);
    void printDecodeValue(int index, MenuItem item);
    int get_indx_obj(int index);
    void clearCursor();
    String floatToConstLen(float input, uint8_t len);
    LiquidCrystal_I2C lcd;
    const MenuItem* menu;
    const Main* lcd_main;
    bool editMode;
    const int mainSize;
    const int menuSize;
    int currentMenu;
    int cursorPos;
    uint8_t scrollPos;
    uint8_t lcdRows;
    uint8_t lcdCols;
};

String leftPadString(String input, int targetLength);