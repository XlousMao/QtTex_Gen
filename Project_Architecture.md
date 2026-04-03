# Anki 数学卡片 Markdown 转换工具 - 技术架构文档

## 1. 项目概述
本项目是一个基于 Qt (C++/PyQt/PySide) 的桌面应用程序。主要功能是读取由大语言模型（如 ChatGPT）按特定格式生成的包含数学公式的 Markdown 文件，解析出独立的 Anki 卡片（包含“文字”、“背面额外”、“标签”），并将其中的 LaTeX 公式（如 `$公式$` 和 `{{c1::$公式$}}`）转换为 Anki 支持的 `[$]公式[/$]` 格式，最终输出一个新的 Markdown 文件供用户直接复制到 Anki。

这非常适合作为 Qt 新手的练手项目，因为它涵盖了 GUI 开发中最核心的几个知识点：**文件 I/O、字符串处理（正则表达式）、数据模型封装、信号与槽机制**。

---

## 2. 核心工作流 (Workflow)
1. **拍照与生成（外部）**：用户拍摄考研复习照片给 AI，AI 按照严格的 Markdown 模板输出知识点文本。
2. **读入（输入阶段）**：用户打开 Qt 程序，通过界面选择加载该 Markdown 文件。
3. **解析（处理阶段 1）**：系统根据分隔符 `---` 将文本切分成多个卡片段落。
4. **提取（处理阶段 2）**：系统通过正则或固定前缀匹配，提取出每张卡片的“文字”、“背面额外”、“标签”。
5. **转换（处理阶段 3）**：对提取出的“文字”与“背面额外”部分，调用正则表达式（借鉴 `test.py`），将数学公式标签转换为 Anki 兼容标签。
6. **导出（输出阶段）**：将转换后的多张卡片重新拼装成规范的 Markdown 文件输出。
7. **手动录入（外部）**：用户打开输出文件，复制内容到 Anki 中（未来可扩展自动导入）。
- **补充内容**：
提示词：
”
**请帮我分析图片中的数学知识点，并提取出高频考点、二级结论或易错陷阱。**

**请务必严格按照以下 Markdown 格式输出，不要改变前后缀，每张卡片之间使用 `---` 分隔，不要输出任何额外的废话，以便我使用程序进行批量正则解析：**

**文字：** [这里填写正面内容，必须使用 $公式$ 或 {{c1::$挖空公式$}} 格式]

**背面额外：** [这里填写原理解释、注意点或留空]

**标签：** [这里填写标签，多个标签用空格分隔，如：数学::微积分 导数]
“
容错性设计
输入异常： 如果用户导入了一个不是按照我们契约生成的 Markdown 文件（比如没有 --- 或者没有 **文字：**），CardParser 应该抛出异常或返回空列表，并在界面上通过 QMessageBox 提示用户“文件格式不符”，而不是让程序直接崩溃。


---

## 3. 项目架构设计

系统设计遵循 **MVC (Model-View-Controller)** 或分层架构思想，将“界面显示”与“核心逻辑”分离。即使将来不使用 Qt，核心解析逻辑也可以直接复用。

### 3.1 目录结构建议
```text
QtTex_Gen/
│
├── core/                   # 核心逻辑层 (完全独立于 UI)
│   ├── AnkiCard.h/cpp      # 数据实体类结构
│   ├── CardParser.h/cpp    # MD 文本解析与切割器
│   ├── LatexConverter.h/cpp# 公式正则转换工具箱
│   └── FileHander.h/cpp    # 文件读写封装
│
├── controller/             # 负责连接 QML 和 Core 逻辑的桥梁
│   └── AppController.h/cpp # 暴露给 QML 的总控类
│
├── ui/                     # 用户界面层 (View - Qt Quick / QML)
│   └── MainWindow.qml      # QML 主窗口界面文件
│
├── CMakeLists.txt          # CMake 构建配置文件
└── main.cpp                # 程序入口 (初始化 QQmlApplicationEngine)
```

---

## 4. 核心类说明 (Class Design)

### 4.1 数据实体：`AnkiCard` (卡片模型)
- **职责**：单纯用来存储一张卡片的三个部分，类似于一个 Struct 或 Data Class。
- **属性**：
  - `QString frontText;` // 文字（正面）
  - `QString backExtra;` // 背面额外
  - `QString tags;`      // 标签
- **方法**：
  - Getter/Setter 方法。
  - `QString toString();` // 将单个卡片按特定格式组装为带界限的字符串。

### 4.2 核心逻辑：`CardParser` (文本解析器)
- **职责**：负责将很长的 Markdown 字符串，切分成一个 `QList<AnkiCard>` 集合。
- **核心方法**：
  - `QList<AnkiCard> parseMarkdown(const QString& rawText);`
- **处理思路**：
  1. 使用 `text.split("---", Qt::SkipEmptyParts)`。
  2. 遍历切割好的片段，利用 `indexOf("**文字：**")`，`indexOf("**背面额外：**")` 和 `indexOf("**标签：**")` 进行字符串切片扫描（String Slicing/Substring），装填成 `AnkiCard` 对象。
- 前面提到使用 indexOf() 进行字符串切片。对于标准化的机器输出，这确实可行，但在实际开发中，AI 输出的文本常常会带有一些不可控的换行符或空格。
- **补充建议**： 
  - 建议从 indexOf 升级为 正则表达式 (QRegularExpression) 提取。
  - 具体思路： 切片出每一段后，使用类似 \*\*文字：\*\*(.*?)\*\*背面额外：\*\* 的正则去捕获内容。同时，在提取出 frontText、backExtra 和 tags 后，一定要调用 QString::trimmed() 清除首尾的空白字符和隐藏换行符，保证存入 AnkiCard 的数据是干净的。

### 4.3 核心逻辑：`LatexConverter` (公式转换器)
- **职责**：将 Python 版本的正则表达式移植过来，将输入文本中的公式转换掉。
- **核心方法**：
  - `QString convertToAnkiLatex(const QString& text);`
- **处理思路**：
  - 定义 `QRegularExpression` 以匹配 `{{c\d::\$...\$}}` 和 `\$...\$`。
  - 使用 Qt 的 `globalMatch` 和迭代替换，或者使用 `QString::replace` 结合正则表达式实现与 Python 中 `re.sub` 一样的效果。核心要处理好 Anki 闭合标签 `}}` 的转义兼容（例如包含分式时 `}}` 变成 `} }`）。
- **补充设计说明**： 
  - 在正则匹配并替换为 [$]...[/$] 之后，必须增加一个循环替换步骤，将捕获到的公式字符串内部所有的 }} 替换为 } }（中间加空格）。

- **开发提示**：
  - 这是为了防止 Anki 填空题的闭合标签 {{c1::...}} 与 LaTeX 分式/根式的连续右大括号产生解析器冲突（Parser Collision）。如果不写进文档，过几天写 C++ 代码时很容易把这个细节忘了。 

### 4.4 核心逻辑：`FileHandler` (文件读写器)
- **职责**：专注于将文本读入内存和将内存文本写回硬盘，解耦 I/O 操作。
- **核心方法**：
  - `QString readFile(const QString& filePath);`
  - `bool saveFile(const QString& filePath, const QString& content);`

### 4.5 用户界面：`MainWindow` (主控界面)
- **职责**：展示按钮、文本框，处理用户的点击交互。
- **核心 UI 组件**：
  - `QPushButton *btnOpenFile`：打开输入 MD 文件。
  - `QPushButton *btnConvert`：开始转换操作。
  - `QPushButton *btnSaveFile`：保存结果到输出 MD 文件。
  - `QTextEdit *textInput`：预览读取到的原始内容。
  - `QTextEdit *textOutput`：预览处理后的结果内容。
- **信号与槽 (Signals & Slots)**：
  - `on_btnOpenFile_clicked()`: 弹出文件选择框获取路径 -> 调用 `FileHandler` -> 显示在左侧文本框。
  - `on_btnConvert_clicked()`: 读取左侧文本 -> 调用 `CardParser` 切分 -> 对所有卡片调用 `LatexConverter` 转换 -> 重新拼接多张卡片 -> 将结果显示在右侧文本框。

---

## 5. 项目后续演进目标 (迭代路线)

- **V 1.0 (当前目标，MVP 极简版)**：
  完成 UI 界面搭建，能手动打开外部 MD 文件，程序正确解析并运用正则替换所有 LaTeX 公式，点击保存输出新的 MD 文件。
- **V 1.1 (体验优化)**：
  增加拖拽文件输入功能 (`dragEnterEvent` & `dropEvent`)；增加状态栏进度提示；增加容错机制（例如 MD 文件格式由于错误没有写 `---` 分隔符时的警告）。
- **V 2.0 (终极自动化)**：
  学习并集成 `AnkiConnect`（Anki 的官方插件，支持本地开启 8765 端口的 http API）。跳过输出 Markdown 文件这一步，在 Qt 中直接发送 POST 请求，拿着解析好的 `(Front, Back, Tags)` 直接静默写入你的 Anki 牌组中。

## 6. 新手开发实操建议
1. **先写核心不用 UI**：可以先用 Qt Console Application 建个小工程，或者直接写个含有 `main` 的类，先尝试把 `QStringList cards = text.split("---")` 这套跑通并在控制台 `qDebug()` 打印。
2. **啃下正则表达式**：C++/Qt 里的 `QRegularExpression` 和 Python 的 `re` 写法类似但 API 不同，可以专门对着 Python 那一段写一个 C++ 的转换函数，给几条假数据测通。
3. **加入界面**：上述核心功能确认有效后，再打开 Qt Designer (或者 Qt Creator) 画几个按钮和文本框，把逻辑挂上去，成就感就会爆棚！