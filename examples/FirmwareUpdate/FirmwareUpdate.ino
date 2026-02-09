/*
  FirmwareUpdate.ino - OTA firmware update decision logic
  
  Demonstrates:
  - Using satisfies() for compatibility checks (NEW!)
  - Firmware version validation
  - Pre-release policy for production devices
  - Update type detection (MAJOR/MINOR/PATCH)
  - Logging with direct printing
  
  Real-world scenario:
  Device checks for firmware updates from a server and decides whether
  to download based on SemVer compatibility rules.
  
  Hardware: ESP32, ESP8266, or any Arduino with network capability
*/

#include <SemVerChecker.h>

// Current firmware version (embedded in device)
const char* CURRENT_VERSION = "2.1.5";

// Minimum compatible firmware version
// Devices older than this MUST upgrade (security fixes, etc.)
const char* MINIMUM_REQUIRED = "2.0.0";

void setup() {
  Serial.begin(115200);
  while(!Serial);

  Serial.println(F("\n=== Firmware Update Check ===\n"));

  // Parse current version
  SemVer current(CURRENT_VERSION);
  Serial.print(F("Current firmware: "));
  Serial.println(current);  // ← Direct print (NEW!)
  
  if (!current.isValid()) {
    Serial.println(F("ERROR: Invalid current version!"));
    return;
  }

  // ============================================
  // Simulate server response
  // ============================================
  // In real application, this would come from HTTP/MQTT
  const char* serverResponse = "2.3.0";  // Simulated API response

  Serial.print(F("Checking server... "));
  Serial.flush();
  delay(500);  // Simulate network delay
  Serial.println(F("OK"));

  // Parse server version
  SemVer available(serverResponse);
  
  if (!available.isValid()) {
    Serial.println(F("✗ ERROR: Invalid version from server"));
    return;
  }

  Serial.print(F("Available firmware: "));
  Serial.println(available);

  // ============================================
  // Pre-release Policy
  // ============================================
  // Production devices skip pre-release versions
  if (available.getPrerelease()[0] != '\0') {
    Serial.print(F("⊘ SKIP: Pre-release version ("));
    Serial.print(available.getPrerelease());
    Serial.println(F(")"));
    Serial.println(F("   Production devices use stable releases only."));
    return;
  }

  // ============================================
  // Compatibility Check (NEW!)
  // ============================================
  // Check if new version is compatible with current
  // Using satisfies() - smarter than simple comparison!
  
  Serial.println(F("\n--- Compatibility Analysis ---"));
  
  if (available <= current) {
    Serial.println(F("✓ Already up to date"));
    return;
  }

  // Check if current version is too old
  SemVer minRequired(MINIMUM_REQUIRED);
  if (!current.satisfies(minRequired)) {
    Serial.println(F("⚠ WARNING: Current version below minimum!"));
    Serial.print(F("   Required: >= "));
    Serial.println(minRequired);
    Serial.println(F("   FORCED UPDATE required"));
  }

  // Check if available version is compatible
  Serial.print(F("Checking compatibility: "));
  Serial.print(available);
  Serial.print(F(" satisfies "));
  Serial.print(current);
  Serial.print(F("? "));

  if (available.satisfies(current)) {
    Serial.println(F("YES ✓"));
    Serial.println(F("   Safe to upgrade (backward compatible)"));
  } else {
    Serial.println(F("NO ✗"));
    Serial.println(F("   Breaking changes detected!"));
    
    // Determine what changed
    SemVer::DiffType diff = current.diff(available);
    
    if (diff == SemVer::MAJOR) {
      Serial.println(F("   • MAJOR version change"));
      Serial.println(F("   • May require user intervention"));
      Serial.println(F("   • Prompt user before updating"));
    }
  }

  // ============================================
  // Update Decision
  // ============================================
  Serial.println(F("\n--- Update Decision ---"));
  
  SemVer::DiffType updateType = current.diff(available);
  
  Serial.print(F("Update type: "));
  switch (updateType) {
    case SemVer::MAJOR:
      Serial.println(F("MAJOR"));
      Serial.println(F("  ⚠ Breaking changes"));
      Serial.println(F("  → Prompt user for confirmation"));
      // showUserPrompt("Major update available. Continue?");
      break;
      
    case SemVer::MINOR:
      Serial.println(F("MINOR"));
      Serial.println(F("  ℹ New features, backward compatible"));
      Serial.println(F("  → Auto-update in background"));
      // scheduleBackgroundUpdate(available);
      break;
      
    case SemVer::PATCH:
      Serial.println(F("PATCH"));
      Serial.println(F("  🔧 Bug fixes, backward compatible"));
      Serial.println(F("  → Auto-update silently"));
      // downloadAndInstall(available);
      break;
      
    default:
      Serial.println(F("UNKNOWN"));
      break;
  }

  // ============================================
  // Simulated Update Process
  // ============================================
  Serial.println(F("\n--- Downloading Update ---"));
  Serial.print(F("Downloading "));
  Serial.print(available);
  Serial.println(F("..."));
  
  // Simulate download progress
  for (int i = 0; i <= 100; i += 20) {
    Serial.print(i);
    Serial.println(F("%"));
    delay(200);
  }
  
  Serial.println(F("✓ Download complete"));
  Serial.println(F("✓ Signature verified"));
  Serial.println(F("ℹ Ready to install (reboot required)"));

  Serial.println(F("\n=== Done ===\n"));
}

void loop() {
  // Nothing to do in loop
}
