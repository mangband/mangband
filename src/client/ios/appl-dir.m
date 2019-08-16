/*
 * This is an Objective-C .m file, that hides away the complexity
 * of NSURL management, and presents a C ABI. See also "appl-dir.h".
 *
 * The code is taken from Apple's developer documentation, with slight
 * modifications.
 */

#import <Foundation/Foundation.h>

void appl_get_appsupport_dir(char* buf, size_t maxlen, int force_create) {
    NSFileManager *fm = [NSFileManager defaultManager];
    NSArray *urls = [fm URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
    NSURL *appSupportDir = [urls firstObject]; // Library/Application Support
    NSString *appBundleID = [[NSBundle mainBundle] bundleIdentifier]; // com.indentifier.some
    NSURL *appDirectory = [appSupportDir URLByAppendingPathComponent:appBundleID];

    const char *cfilename = [appDirectory fileSystemRepresentation];
    strlcpy(buf, cfilename, maxlen);
    
    NSError *error;
    if (force_create && ![fm createDirectoryAtPath:[appDirectory path]
                            withIntermediateDirectories:YES
                            attributes:nil
                            error:&error])
    {
        NSLog(@"Create directory error: %@", error);
    }
}

void appl_get_documents_dir(char* buf, size_t maxlen) {
    NSFileManager *fm = [NSFileManager defaultManager];
    NSArray *urls = [fm URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask];
    NSURL *docsDir = [urls firstObject];

    const char *cfilename = [docsDir fileSystemRepresentation];
    strlcpy(buf, cfilename, maxlen);
}
