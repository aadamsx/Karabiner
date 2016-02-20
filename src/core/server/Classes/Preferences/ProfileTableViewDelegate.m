#import "PreferencesManager.h"
#import "ProfileCellView.h"
#import "ProfileTableViewDelegate.h"
#import "ServerObjects.h"

@interface ProfileTableViewDelegate ()
@property(weak) IBOutlet ServerObjects* serverObjects;
@end

@implementation ProfileTableViewDelegate

- (NSView*)tableView:(NSTableView*)tableView viewForTableColumn:(NSTableColumn*)tableColumn row:(NSInteger)row {
  ProfileCellView* result = [tableView makeViewWithIdentifier:@"ProfileCellView" owner:self];
  result.serverObjects = self.serverObjects;
  result.profileIndex = row;

  NSString* name = [self.serverObjects.preferencesManager configlist_name:row];
  if (name) {
    result.textField.stringValue = name;
  }

  if ([self.serverObjects.preferencesManager configlist_selectedIndex] == row) {
    result.deleteButton.hidden = YES;
    result.statusLabel.hidden = NO;
  } else {
    result.deleteButton.hidden = NO;
    result.statusLabel.hidden = YES;
  }

  return result;
}

@end