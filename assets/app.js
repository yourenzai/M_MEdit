// M_MEdit 核心前端逻辑

document.addEventListener('DOMContentLoaded', () => {
    const editor = document.getElementById('editor');
    const previewContent = document.getElementById('preview-content');
    const menubar = document.getElementById('menubar');
    const menuItems = document.querySelectorAll('.menu-item');
    const workbench = document.querySelector('.workbench');

    // 状态栏元素
    const cursorPosEl = document.getElementById('cursor-pos');
    const charCountEl = document.getElementById('char-count');
    const statusDirtyEl = document.getElementById('status-dirty');

    let isMenuOpen = false; // 是否已有菜单处于展开状态

    // ===== 1. 顶部菜单栏交互 (Typora 交互机制) =====
    menuItems.forEach(item => {
        const title = item.querySelector('.menu-title');

        // 点击打开/切换
        title.addEventListener('click', (e) => {
            e.stopPropagation();
            if (item.classList.contains('open')) {
                closeAllMenus();
            } else {
                closeAllMenus();
                item.classList.add('open');
                isMenuOpen = true;
            }
        });

        // 当已经有菜单展开时，滑入其他菜单项立即无缝切换
        item.addEventListener('mouseenter', () => {
            if (isMenuOpen && !item.classList.contains('open')) {
                closeAllMenus();
                item.classList.add('open');
            }
        });
    });

    function closeAllMenus() {
        menuItems.forEach(i => i.classList.remove('open'));
        isMenuOpen = false;
    }

    // 点击外部区域或编辑器关闭菜单
    document.addEventListener('click', () => {
        if (isMenuOpen) closeAllMenus();
    });

    // 按 ESC 键关闭菜单
    window.addEventListener('keydown', (e) => {
        if (e.key === 'Escape') closeAllMenus();
    });

    // ===== 2. 菜单动作命令分发 =====
    const menuActions = document.querySelectorAll('.menu-action');
    menuActions.forEach(action => {
        action.addEventListener('click', (e) => {
            e.stopPropagation();
            const cmd = action.getAttribute('data-command');
            handleCommand(cmd);
            closeAllMenus();
        });
    });

    function handleCommand(cmd) {
        switch (cmd) {
            // 文件命令
            case 'file-new':
                editor.value = '';
                updatePreview();
                break;
            case 'file-save':
                if (window.chrome && window.chrome.webview) {
                    window.chrome.webview.postMessage({ type: 'save', content: editor.value });
                } else {
                    console.log('保存内容:', editor.value);
                }
                statusDirtyEl.textContent = '已保存';
                break;

            // 格式化命令
            case 'fmt-bold':
                wrapSelection('**', '**');
                break;
            case 'fmt-italic':
                wrapSelection('*', '*');
                break;
            case 'fmt-underline':
                wrapSelection('<u>', '</u>');
                break;
            case 'fmt-strike':
                wrapSelection('~~', '~~');
                break;
            case 'fmt-code':
                wrapSelection('`', '`');
                break;
            case 'fmt-link':
                wrapSelection('[', '](https://)');
                break;
            case 'fmt-image':
                wrapSelection('![alt](', ')');
                break;

            // 段落命令
            case 'para-h1':
                insertLinePrefix('# ');
                break;
            case 'para-h2':
                insertLinePrefix('## ');
                break;
            case 'para-h3':
                insertLinePrefix('### ');
                break;
            case 'para-h4':
                insertLinePrefix('#### ');
                break;
            case 'para-ul':
                insertLinePrefix('- ');
                break;
            case 'para-ol':
                insertLinePrefix('1. ');
                break;
            case 'para-quote':
                insertLinePrefix('> ');
                break;
            case 'para-codeblock':
                wrapSelection('```\n', '\n```');
                break;
            case 'para-hr':
                insertAtCursor('\n---\n');
                break;

            // 视图命令
            case 'view-edit':
                workbench.className = 'workbench view-mode-edit';
                break;
            case 'view-split':
                workbench.className = 'workbench';
                break;
            case 'view-preview':
                workbench.className = 'workbench view-mode-preview';
                break;

            // 主题命令
            case 'theme-light':
                document.body.removeAttribute('data-theme');
                break;
            case 'theme-dark':
                document.body.setAttribute('data-theme', 'dark');
                break;

            // 帮助命令
            case 'help-about':
                alert('M_MEdit - 轻量现代 Markdown 编辑器\n基于 Win32 + WebView2 构建');
                break;
            case 'help-markdown':
                editor.value = `# 欢迎使用 M_MEdit\n\n这是一款轻量现代的 **Markdown 编辑器**。\n\n## 功能特性\n[x] 实时双栏预览\n- [x] 原生快捷键支持\n\n> 享受极简创作乐趣！`;
                updatePreview();
                break;
        }
    }

    // ===== 3. 编辑器文本操作辅助函数 =====
    function wrapSelection(prefix, suffix) {
        editor.focus();
        const start = editor.selectionStart;
        const end = editor.selectionEnd;
        const selText = editor.value.substring(start, end);
        const replacement = prefix + (selText || '文本') + suffix;

        editor.setRangeText(replacement, start, end, 'select');
        updatePreview();
    }

    function insertLinePrefix(prefix) {
        editor.focus();
        const start = editor.selectionStart;
        const val = editor.value;
        const lineStart = val.lastIndexOf('\n', start - 1) + 1;

        editor.value = val.substring(0, lineStart) + prefix + val.substring(lineStart);
        editor.selectionStart = editor.selectionEnd = start + prefix.length;
        updatePreview();
    }

    function insertAtCursor(text) {
        editor.focus();
        const start = editor.selectionStart;
        editor.setRangeText(text, start, editor.selectionEnd, 'end');
        updatePreview();
    }

    // ===== 4. 实时 Markdown 预览简易渲染 =====
    function renderSimpleMarkdown(md) {
        if (!md.trim()) {
            return '<p style="color: #8c959f;">在左侧输入 Markdown，右侧将实时渲染预览...</p>';
        }

        // 简易 Markdown 转 HTML（后续可引入 marked.js）
        let html = md
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;');

        // 标题
        html = html.replace(/^### (.*$)/gim, '<h3>$1</h3>');
        html = html.replace(/^## (.*$)/gim, '<h2>$1</h2>');
        html = html.replace(/^# (.*$)/gim, '<h1>$1</h1>');

        // 加粗、斜体、删除线、行内代码
        html = html.replace(/\*\*(.*?)\*\*/g, '<strong>$1</strong>');
        html = html.replace(/\*(.*?)\*/g, '<em>$1</em>');
        html = html.replace(/~~(.*?)~~/g, '<del>$1</del>');
        html = html.replace(/`([^`]+)`/g, '<code>$1</code>');

        // 引用块与分割线
        html = html.replace(/^> (.*$)/gim, '<blockquote>$1</blockquote>');
        html = html.replace(/^---$/gim, '<hr>');

        // 换行与段落
        html = html.split('\n\n').map(p => {
            if (p.startsWith('<h') || p.startsWith('<blockquote') || p.startsWith('<hr')) return p;
            return `<p>${p.replace(/\n/g, '<br>')}</p>`;
        }).join('');

        return html;
    }

    function updatePreview() {
        previewContent.innerHTML = renderSimpleMarkdown(editor.value);
        updateStatus();
    }

    // ===== 5. 状态栏更新与快捷键绑定 =====
    function updateStatus() {
        charCountEl.textContent = `${editor.value.length} 字`;
        statusDirtyEl.textContent = '已修改';

        // 光标行列计算
        const textBeforeCursor = editor.value.substring(0, editor.selectionStart);
        const lines = textBeforeCursor.split('\n');
        const line = lines.length;
        const col = lines[lines.length - 1].length + 1;
        cursorPosEl.textContent = `第 ${line} 行, 第 ${col} 列`;
    }

    editor.addEventListener('input', updatePreview);
    editor.addEventListener('keyup', updateStatus);
    editor.addEventListener('click', updateStatus);

    // 快捷键支持 (Ctrl+B, Ctrl+I, Ctrl+S 等)
    editor.addEventListener('keydown', (e) => {
        if (e.ctrlKey) {
            switch (e.key.toLowerCase()) {
                case 's':
                    e.preventDefault();
                    handleCommand('file-save');
                    break;
                case 'b':
                    e.preventDefault();
                    handleCommand('fmt-bold');
                    break;
                case 'i':
                    e.preventDefault();
                    handleCommand('fmt-italic');
                    break;
                case 'k':
                    e.preventDefault();
                    handleCommand('fmt-link');
                    break;
                case '/':
                    e.preventDefault();
                    handleCommand('view-split');
                    break;
            }
        }
    });

    // 初始渲染默认欢迎文档
    handleCommand('help-markdown');
});
