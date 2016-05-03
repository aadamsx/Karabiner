#import "PreferencesModel.h"

@implementation ProfileModel

#pragma mark - NSObject

- (id)replacementObjectForPortCoder:(NSPortCoder*)encoder {
  if ([encoder isBycopy]) return self;
  return [super replacementObjectForPortCoder:encoder];
}

#pragma mark - NSCoding

- (instancetype)initWithCoder:(NSCoder*)decoder {
  self = [super init];

  if (self) {
    self.name = [decoder decodeObjectForKey:@"name"];
    self.identifier = [decoder decodeObjectForKey:@"identifier"];
    self.appendIndex = [decoder decodeIntegerForKey:@"appendIndex"];
  }

  return self;
}

- (void)encodeWithCoder:(NSCoder*)encoder {
  [encoder encodeObject:self.name forKey:@"name"];
  [encoder encodeObject:self.identifier forKey:@"identifier"];
  [encoder encodeInteger:self.appendIndex forKey:@"appendIndex"];
}

- (id)copyWithZone:(NSZone*)zone {
  ProfileModel* obj = [[[self class] allocWithZone:zone] init];
  if (obj) {
    obj.name = [self.name copyWithZone:zone];
    obj.identifier = [self.identifier copyWithZone:zone];
    obj.appendIndex = self.appendIndex;
  }
  return obj;
}

@end

@implementation AXNotifierPreferencesModel

- (void)log {
  NSMutableString* string = [NSMutableString new];
  [string appendString:@"AXNotifierPreferencesModel:\n"];
  [string appendString:@"{\n"];

#define LOG(KEY)                                           \
  {                                                        \
    [string appendFormat:@"    %s: %d\n", #KEY, self.KEY]; \
  }

  LOG(useAXNotifier);
  LOG(disableAXNotifierInJavaApps);
  LOG(disableAXNotifierInQtApps);
  LOG(disableAXNotifierInPreview);
  LOG(disableAXNotifierInMicrosoftOffice);

  [string appendString:@"}\n"];

  NSLog(@"%@", string);
}

#pragma mark - NSObject

- (id)replacementObjectForPortCoder:(NSPortCoder*)encoder {
  if ([encoder isBycopy]) return self;
  return [super replacementObjectForPortCoder:encoder];
}

#pragma mark - NSCoding

- (instancetype)initWithCoder:(NSCoder*)decoder {
  self = [super init];

  if (self) {
#define DECODE(KEY)                               \
  {                                               \
    self.KEY = [decoder decodeBoolForKey:@ #KEY]; \
  }

    DECODE(useAXNotifier);
    DECODE(disableAXNotifierInJavaApps);
    DECODE(disableAXNotifierInQtApps);
    DECODE(disableAXNotifierInPreview);
    DECODE(disableAXNotifierInMicrosoftOffice);
  }

  return self;
}

- (void)encodeWithCoder:(NSCoder*)encoder {
#define ENCODE(KEY)                              \
  {                                              \
    [encoder encodeBool:self.KEY forKey:@ #KEY]; \
  }

  ENCODE(useAXNotifier);
  ENCODE(disableAXNotifierInJavaApps);
  ENCODE(disableAXNotifierInQtApps);
  ENCODE(disableAXNotifierInPreview);
  ENCODE(disableAXNotifierInMicrosoftOffice);
}

- (id)copyWithZone:(NSZone*)zone {
  AXNotifierPreferencesModel* obj = [[[self class] allocWithZone:zone] init];
  if (obj) {
#define COPY(KEY)       \
  {                     \
    obj.KEY = self.KEY; \
  }

    COPY(useAXNotifier);
    COPY(disableAXNotifierInJavaApps);
    COPY(disableAXNotifierInQtApps);
    COPY(disableAXNotifierInPreview);
    COPY(disableAXNotifierInMicrosoftOffice);
  }
  return obj;
}

@end

@implementation PreferencesModel

- (void)addProfile:(NSString*)name {
}

- (void)sortProfilesByAppendIndex {
}

- (void)sortProfilesByName {
}

@end
