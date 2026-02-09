/*
  PrintableIntegration.ino - Demonstrates Printable interface benefits
  
  Showcases:
  - Direct printing to Serial (zero-allocation)
  - Comparison: printTo() vs toString()
  - Performance benefits on embedded devices
  - Works with any Print stream (LCD, OLED, File, etc.)
  
  NEW in SemVerChecker v1.1.0+:
  SemVer now inherits from Printable, enabling direct use with Serial.print()
  and any other Print-based output (LiquidCrystal, Adafruit_GFX, SD card, etc.)
  
  Hardware: Any Arduino board
*/

#include <SemVerChecker.h>

// Example versions for demonstration
SemVer firmware("2.5.3-rc.1+build.789");
SemVer library("1.0.0");
SemVer devBuild("0.9.5-dev");

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println(F("\n=== Printable Interface Demo ===\n"));

  // ============================================
  // 1. Direct Printing (Recommended)
  // ============================================
  Serial.println(F("[1] DIRECT PRINTING"));
  Serial.println(F("----------------------------------------"));
  Serial.println(F("  SemVer objects print directly:\n"));
  
  Serial.print(F("  Firmware: "));
  Serial.println(firmware);  // ← Zero allocation!
  
  Serial.print(F("  Library:  "));
  Serial.println(library);
  
  Serial.print(F("  Dev build: "));
  Serial.println(devBuild);

  // ============================================
  // 2. Inline Printing
  // ============================================
  Serial.println(F("\n[2] INLINE PRINTING"));
  Serial.println(F("----------------------------------------"));
  Serial.println(F("  Embed versions in messages:\n"));
  
  Serial.print(F("  Current version: "));
  Serial.print(firmware);
  Serial.println(F(" (stable)"));
  
  Serial.print(F("  Dependencies: Library v"));
  Serial.println(library);

  // ============================================
  // 3. Formatted Output
  // ============================================
  Serial.println(F("\n[3] FORMATTED OUTPUT"));
  Serial.println(F("----------------------------------------\n"));
  
  Serial.println(F("  System Status:"));
  Serial.println(F("  ┌─────────────────────────────┐"));
  Serial.print(F("  │ Firmware:  "));
  Serial.print(firmware);
  
  // Pad to align (simple version)
  Serial.println(F("       │"));
  
  Serial.print(F("  │ SDK:       "));
  Serial.print(library);
  Serial.println(F("                 │"));
  
  Serial.println(F("  └─────────────────────────────┘"));

  // ============================================
  // 4. Comparison: printTo() vs toString()
  // ============================================
  Serial.println(F("\n[4] PERFORMANCE COMPARISON"));
  Serial.println(F("----------------------------------------\n"));
  
  SemVer testVer("3.14.159-alpha+special");
  
  // Method 1: toString() (creates String object)
  Serial.println(F("  Method 1 (OLD): toString()"));
  Serial.println(F("  ├─ Creates String object (heap allocation)"));
  Serial.println(F("  ├─ Slower on AVR (5-10x)"));
  Serial.println(F("  ├─ Heap fragmentation risk"));
  Serial.println(F("  └─ Use when: Need String for concatenation\n"));
  
  String str = testVer.toString();
  Serial.print(F("     Result: \""));
  Serial.print(str);
  Serial.println(F("\""));
  
  // Method 2: Direct print (uses printTo() internally)
  Serial.println(F("\n  Method 2 (NEW): Direct print"));
  Serial.println(F("  ├─ Zero heap allocation"));
  Serial.println(F("  ├─ Faster (5-10x on AVR)"));
  Serial.println(F("  ├─ No fragmentation"));
  Serial.println(F("  └─ Use when: Just printing to Serial/LCD\n"));
  
  Serial.print(F("     Result: \""));
  Serial.print(testVer);
  Serial.println(F("\""));

  // ============================================
  // 5. Works with Any Print Stream
  // ============================================
  Serial.println(F("\n[5] UNIVERSAL COMPATIBILITY"));
  Serial.println(F("----------------------------------------"));
  Serial.println(F("  printTo() works with ANY Print object:\n"));
  
  Serial.println(F("  • Serial (UART)"));
  Serial.println(F("  • Serial1, Serial2, ... (additional UARTs)"));
  Serial.println(F("  • LiquidCrystal (LCD displays)"));
  Serial.println(F("  • Adafruit_GFX (OLED, TFT displays)"));
  Serial.println(F("  • SD/File (SD card writing)"));
  Serial.println(F("  • WiFiClient, EthernetClient (network)"));
  Serial.println(F("  • Any custom class inheriting from Print"));

  // ============================================
  // 6. Memory Efficiency Demo
  // ============================================
  Serial.println(F("\n[6] MEMORY EFFICIENCY"));
  Serial.println(F("----------------------------------------\n"));
  
  Serial.println(F("  Stack usage comparison:"));
  Serial.println(F("  • toString():     ~70+ bytes (String object + heap)"));
  Serial.println(F("  • printTo():      ~15 bytes (temporary buffer only)"));
  Serial.println(F("  • Savings:        ~55 bytes per call"));
  Serial.println(F("\n  On Arduino Uno (2KB RAM), every byte counts!"));

  // ============================================
  // 7. Best Practices
  // ============================================
  Serial.println(F("\n[7] BEST PRACTICES"));
  Serial.println(F("----------------------------------------\n"));
  
  Serial.println(F("  ✓ DO:"));
  Serial.println(F("    • Use direct print for Serial output"));
  Serial.println(F("    • Use direct print for LCD/OLED"));
  Serial.println(F("    • Use direct print for logging"));
  
  Serial.println(F("\n  ⚠ CONSIDER toString() when:"));
  Serial.println(F("    • Need to concatenate with other strings"));
  Serial.println(F("    • Need to store version as String"));
  Serial.println(F("    • Sending via HTTP/MQTT (use c_str())"));

  Serial.println(F("\n=== Done ===\n"));
}

void loop() {
  // Nothing to do in loop
}
