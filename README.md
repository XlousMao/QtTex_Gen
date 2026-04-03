<div align="center">

# 🚀 QtTex_Gen (Anki Math Card Converter) v2.0

**极致优雅的 Markdown -> Anki 数学/理科闪卡自动化构建工具**

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=flat-square&logo=c%2B%2B)
![Qt6](https://img.shields.io/badge/Qt-6.8-41CD52.svg?style=flat-square&logo=qt)
![CMake](https://img.shields.io/badge/CMake-3.16+-064F8C.svg?style=flat-square&logo=cmake)
![License](https://img.shields.io/badge/License-MIT-green.svg?style=flat-square)

</div>

## 🌟 项目简介

**QtTex_Gen** 是一款专为理科生、医学生及高级 Anki 玩家打造的开源桌面软件。它的核心使命是：**将 AI（如 Gemini、ChatGPT、Claude）生成的包含复杂 LaTeX 公式的 Markdown 笔记，一键极速转化为完美的 Anki 填空卡（Cloze）并直接跨进程无缝写入 Anki 数据库。**

不再需要繁琐的复制粘贴，不再面临 Anki 渲染 `\frac{}` 时与双大括号 `{{ }}` 冲突导致报错退出的恶心体验！

### 🔥 v2.0 重磅升级史诗级特性
1. **AnkiConnect 原生集成**：完全抛弃中转文本！现在基于 HTTP JSON RPC 直接与后台 Anki 进程通信，自动拉取你的**全量牌组**、**卡片模板**与**字段清单**，实现真正的**一键制卡**！
2. **全新 Clash-Verge 风格 UI**：重构了 QML 界面，采用极简浅灰背景、现代化悬浮白净面板、悬停过渡动效，享受视觉与体验的双重愉悦。
3. **高并发与自动排错**：基于 `QNetworkAccessManager` 的异步网络流处理，全自动容错（空字段补全、去重拦截提示）。

---

## ✨ 核心特性

- 🧠 **AI 友好型解析引擎**：无论是 `**文字：**` 还是带有额外空格的落单 `$` 符号，C++ 正则引擎都能精确提取正面、背面和标签。
- 🛡️ **大括号防碰撞 (Brace Collision Fix)**：独创算法，精准切割 `{{c1:: ... }}` 填空符与深层嵌套的 LaTeX 语法，确保 `\frac{e^x}{2}` 这类公式在 Anki 中被完美渲染。
- ⚡ **现代化 C++ & Qt6 架构**：
  - **后端**：纯 C++17 编写，极速解析。使用 `AnkiConnect` 类接管底层网络通信封装。
  - **中间层**：`AppController` 采用 MVC 模式的信号槽机制，与 QML UI 高效桥接。
  - **前端**：Qt Quick / QML 构建，响应式分栏布局。

---

## 🛠️ 项目架构

```text
📦 QtTex_Gen
 ┣ 📂 AnkiConnect    # (v2.0+) 基于 QNetworkAccessManager 的 JSON RPC 网络通信库
 ┣ 📂 controller     # AppController: 连接 C++ 核心业务与 QML UI 的桥梁
 ┣ 📂 core           # 核心引擎: CCardParser(正则切割), CLatexConverter(公式包裹与冲突修复)
 ┣ 📂 ui             # MainWindow.qml 等全新前端界面代码
 ┗ 📜 CMakeLists.txt # 现代 CMake 脚本（支持自动提取 Qt 部署库 windeployqt）
```

---

## 📝 支持的 Markdown 输入格式要求

使用 AI 生成笔记时，请让 AI 输出如下标准格式（不同的卡片之间用 `---` 隔开）：

```markdown
文字： 悬链线方程（取 $a=1$ 时）的表达式为 $y = $ {{c1::\frac{e^x + e^{-x}}{2}}}，它是一个 {{c2::偶}} 函数。

背面额外： 该函数又被称为双曲余弦函数 $\cosh x$，在物理中常用于描述两端固定的均匀柔性链条。

标签： 数学::函数性质
---
文字： 极限 $\lim_{x \to \infty} e^x$ 的结果是 {{c1::不存在}}。

背面额外： 【易错陷阱】必须区分趋向，方向不同结果不同。

标签： 数学::高数
```
推荐的AI提示词：
```markdown
请帮我分析图片的数学/专业课知识点，提取出高频考点、定理公式、二级结论或易错陷阱，并帮我制作成 Anki 记忆卡片。

【严格的输出格式限制】
为了方便我使用自己开发的程序进行批量正则解析，请你**务必绝对遵守**以下 Markdown 文本输出格式。

1. 不要输出任何开场白、解释性废话或总结语，直接输出卡片内容。
2. 卡片与卡片之间必须仅使用独立的 `---` 独占一行进行分隔。
3. 必须严格使用 `**文字：**`、`**背面额外：**`、`**标签：**` 作为每一项的开头，不可省略或更改任何符号。
4. 所有的数学公式必须使用 `$公式内容$` 包裹。如果正面需要作为“填空题”挖空测试，请严格使用 `{{c1::$挖空公式$}}` 的格式。

【卡片结构模板】
**文字：** [这里填写正面提问或带填空的定理]
**背面额外：** [这里填写原理解释、注意点或推导逻辑。如果无内容可写也必须保留标题并留白]
**标签：** [填写标签，例如：数学::微积分]

【标准输出示例】
**文字：** 一元二次方程 $ax^2+bx+c=0$ 的求根公式是 {{c1::$\frac{-b \pm \sqrt{b^2-4ac}}{2a}$}}
**背面额外：** 注意前提条件是 $\Delta = b^2-4ac \ge 0$ 且 $a \neq 0$。
**标签：** 数学::代数
---

**文字：** 什么是牛顿-莱布尼茨公式（微积分基本定理）？
**背面额外：** $\int_a^b f(x)dx = F(b) - F(a)$，其中 $F(x)$ 是 $f(x)$ 的一个原函数。
**标签：** 数学::微积分
---
```


---

## ⚡ 快速开始

### 1. 准备环境
- **安装 Anki 及 插件**：在你的 Anki 中安装 **AnkiConnect** 插件 (代码: `2055492159`)。保持 Anki 处于打开状态。
- **配置 AnkiConnect** (可选但推荐): 在插件设置中允许你的应用访问（通常默认 `127.0.0.1` 即可）。

### 2. 编译本项目
```bash
git clone https://github.com/yourusername/QtTex_Gen.git
cd QtTex_Gen
mkdir build && cd build
cmake -G Ninja ..
cmake --build .
```
*(注：Windows 环境下 CMake 会自动调用 `windeployqt` 将所需依赖拷贝至同目录，做到即编即用！)*

### 3. 开始制卡
1. 启动软件，你会立刻看到右侧日志显示 **`>>> [API] Connected to Anki Connect.`**，并且自动拉取了你的牌组。
2. 点击 **Play Open MD File** 加载你准备好的 Markdown 笔记。
3. 从下拉菜单选择**目标牌组 (Deck)** 和 **卡片模板 (Model)**（推荐使用“填空题”或“Cloze”模板）。
4. 点击闪亮的蓝底按钮 **Import directly to Anki**，不到一秒钟，数十张满载华丽 LaTeX 公式的卡片已经躺在你的 Anki 复习列队中！

---

## 📈 Star History

[![Star History Chart](https://api.star-history.com/svg?repos=yourusername/QtTex_Gen&type=Date)](https://star-history.com/#yourusername/QtTex_Gen&Date)

> **小提示**：如果你喜欢这个项目，欢迎点击右上角的 ⭐ **Star**！你的支持是我更新和维护的最大动力！

---

## 🤝 贡献与反馈
有任何绝妙的 Idea 或者大 Bug？欢迎提 Issue 或 Pull Request！我们非常乐意合并让应用变得更好用的代码。

## 📜 许可证 (License)
本项目遵守 [MIT License](LICENSE) 开源协议。你可以自由地将其用于你的个人学习流、整合甚至二次开发。