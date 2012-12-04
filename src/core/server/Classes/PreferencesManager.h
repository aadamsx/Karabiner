// -*- Mode: objc; Coding: utf-8; indent-tabs-mode: nil; -*-

#import <Cocoa/Cocoa.h>

@class ClientForKernelspace;

@interface PreferencesManager : NSObject {
  NSMutableDictionary* default_;
  NSArray* essential_configuration_identifiers_;

  IBOutlet ClientForKernelspace* clientForKernelspace_;
}

- (void) load;

- (int) value:(NSString*)name;
- (int) defaultValue:(NSString*)name;
- (void) setValueForName:(int)newval forName:(NSString*)name;

- (NSArray*) essential_config;
- (NSDictionary*) changed;

// --------------------------------------------------
- (NSInteger)     configlist_selectedIndex;
- (NSString*)     configlist_selectedName;
- (NSString*)     configlist_selectedIdentifier;
- (NSArray*)      configlist_getConfigList;
- (NSUInteger)    configlist_count;
- (NSDictionary*) configlist_dictionary:(NSInteger)rowIndex;
- (NSString*)     configlist_name:(NSInteger)rowIndex;
- (NSString*)     configlist_identifier:(NSInteger)rowIndex;
- (void)          configlist_select:(NSInteger)newindex;
- (void)          configlist_setName:(NSInteger)rowIndex name:(NSString*)name;
- (void)          configlist_append;
- (void)          configlist_delete:(NSInteger)rowIndex;

- (NSInteger) checkForUpdatesMode;

- (IBAction) sendConfigListChangedNotification:(id)sender;

@end
