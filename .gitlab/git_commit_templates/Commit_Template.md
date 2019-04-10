【提交分类】简洁描述修改摘要
> 提交分类说明
>【Bugfix-BugID】修复Bugfree提出的bug，带上Bug ID
>【Bugfix】修复开发中自测出现的问题
>【Feature】添加新的Feature
>【Optimize】重构代码，优化结构等
>【Resources】修改资源，替换翻译等，不涉及代码修改的
>【Demo】只更新了Demo，未涉及源码修改。
>【Doc】Readme、Changelog 等文档文件的整理和修改。
>【Podspecs】podspecs文件修改，如果是版本封包需要带上-version
> 如果提交 A 依赖于另一个提交 B，当提交时，应在提交 A 使用提交 B 的Hash在消息中说明该依赖项；同样地，如果提交一个由提交B引入的 bug，那么它应该在提交 A 中提出。

详细的描述你的修改。
并且解释了为什么需要这么修改，如何解决这个问题。它可能有什么副作用。

【影响范围】XXXXXX
【引入】引入这个bug的commit的 SHA，不需要完整的SHA，取前7位
【依赖】如果提交 A 依赖于另一个提交 B，当提交时，应在提交 A 使用提交 B 的SHA在消息中说明该依赖项，不需要完整的SHA，取前7位
【URL】引用 URL 或 Jira 的 URL

> 更多细节参考[iOS Git Commit 规范](http://cf.meitu.com/confluence/pages/viewpage.action?pageId=4556189)
