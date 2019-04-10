//
//  ViewController.m
//  MTAppenderFileDemo
//
//  Created by cqh on 18/05/2017.
//  Copyright © 2017 Meitu. All rights reserved.
//

#import "ViewController.h"
#import <MTAppenderFile/MTAppenderFile.h>

@interface ViewController ()
@property (nonatomic, strong) MTAppenderFile *logger;
@property (nonatomic, strong) NSTimer *timer;
@property (nonatomic, strong) NSMutableArray *loggers;
@property (nonatomic, strong) MTAppenderFile *testLogger;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    NSString *logPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    self.logger = [[MTAppenderFile alloc] initWithFileDir:logPath name:@"test"];
    [self.logger open];

    UIButton *btn1 = [[UIButton alloc] initWithFrame:CGRectMake(100, 100, 100, 40)];
    [btn1 setTitle:@"new logger" forState:UIControlStateNormal];
    [btn1 setTitleColor:[UIColor blueColor] forState:UIControlStateNormal];
    [btn1 addTarget:self action:@selector(addLog) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:btn1];

    UIButton *closeBtn = [[UIButton alloc] initWithFrame:CGRectMake(100, 200, 100, 40)];
    [closeBtn setTitle:@"close" forState:UIControlStateNormal];
    [closeBtn setTitleColor:[UIColor redColor] forState:UIControlStateNormal];
    [closeBtn addTarget:self action:@selector(close) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:closeBtn];
    
    UIButton *writeSomeTestBtn = [[UIButton alloc] initWithFrame:CGRectMake(100, 250, 100, 40)];
    [writeSomeTestBtn setTitle:@"writeFileTest" forState:UIControlStateNormal];
    [writeSomeTestBtn setTitleColor:[UIColor redColor] forState:UIControlStateNormal];
    [writeSomeTestBtn addTarget:self action:@selector(writeSomeTestEvent) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:writeSomeTestBtn];

    self.timer = [NSTimer scheduledTimerWithTimeInterval:1 target:self selector:@selector(test) userInfo:nil repeats:YES];
}

- (MTAppenderFile *)testLogger {
    if (!_testLogger) {
        NSString *logPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
        _testLogger = [[MTAppenderFile alloc] initWithFileDir:logPath name:@"records"];
    }
    return _testLogger;
}

- (NSMutableArray *)loggers {
    if (!_loggers) {
        _loggers = [[NSMutableArray alloc] init];
    }
    return _loggers;
}

- (void)test {
    NSLog(@"填充Log");
    @synchronized(self) {
        for (MTAppenderFile *logger in self.loggers) {
            [logger appendText:[NSString stringWithFormat:@"%lu: %s", (unsigned long)[self.loggers indexOfObject:logger], __PRETTY_FUNCTION__]];
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(void) {
                for (int i = 0; i < 1000; i++) {
                    [logger appendText:@"dsajkdflsafkdslajfdlsafjdslkahfdklsa\n"];
                }
            });
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^(void) {
                for (int i = 0; i < 1000; i++) {
                    [logger appendText:@"11132413501350\n"];
                }
            });
        }
    }
}

- (void)addLog {
    NSLog(@"添加一个MTAppenderFile");
    @synchronized(self) {
        static int index = 0;
        NSString *documentPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
        NSString *logFileName = [NSString stringWithFormat:@"log_%i", index++];
        MTAppenderFile *logger = [[MTAppenderFile alloc] initWithFileDir:documentPath name:logFileName];
        [logger open];
        [self.loggers addObject:logger];
    }
}

- (void)close {
    NSLog(@"关闭所有MTAppenderFile");
    @synchronized(self) {
        for (MTAppenderFile *logger in self.loggers) {
            [logger close];
        }
        [self.loggers removeAllObjects];
    }
}

- (void)writeSomeTestEvent {
    [self.testLogger open];
    [self.testLogger appendText:@"11111111"];
    [self.testLogger appendText:@"22222222"];
    [self.testLogger appendText:@"33333333"];
    [self.testLogger appendText:@"44444444"];
    [self.testLogger appendText:@"55555555"];
    [self.testLogger appendText:@"66666666"];
    [self.testLogger appendText:@"77777777"];
    [self.testLogger close];
    NSLog(@"done");
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
