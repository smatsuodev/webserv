(function () {
    const form = document.getElementById('uploadForm');
    const input = document.getElementById('fileInput');
    const dropzone = document.getElementById('dropzone');
    const fileList = document.getElementById('fileList');
    const bar = document.getElementById('bar');
    const resp = document.getElementById('response');
    const submitBtn = document.getElementById('submitBtn');

    function listFiles(files) {
        if (!files || files.length === 0) {
            fileList.textContent = '';
            return;
        }
        const items = Array.from(files).map(f => `${f.name} (${(f.size / 1024).toFixed(1)} KB)`);
        fileList.textContent = items.join('\n');
    }

    // ドロップゾーンの操作
    function preventDefaults(e) {
        e.preventDefault();
        e.stopPropagation();
    }

    ['dragenter', 'dragover', 'dragleave', 'drop'].forEach(ev => {
        dropzone.addEventListener(ev, preventDefaults, false);
    });
    ['dragenter', 'dragover'].forEach(ev => {
        dropzone.addEventListener(ev, () => dropzone.classList.add('dragover'), false);
    });
    ['dragleave', 'drop'].forEach(ev => {
        dropzone.addEventListener(ev, () => dropzone.classList.remove('dragover'), false);
    });
    dropzone.addEventListener('click', () => input.click());
    dropzone.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' || e.key === ' ') {
            e.preventDefault();
            input.click();
        }
    });
    dropzone.addEventListener('drop', (e) => {
        const dt = e.dataTransfer;
        if (dt && dt.files && dt.files.length) {
            input.files = dt.files; // 一括選択
            listFiles(input.files);
        }
    });
    input.addEventListener('change', () => listFiles(input.files));

    // 非同期アップロード（XHR: 進捗取得用）
    form.addEventListener('submit', function (e) {
        // JavaScript 有効時は XHR で送信（進捗表示）
        e.preventDefault();
        resp.textContent = '送信中…';
        bar.style.width = '0%';
        submitBtn.disabled = true;

        const fd = new FormData();
        // CSRF など他のフィールドを含めたい場合は form.elements を走査
        // ここではファイルのみ送信
        if (input.files && input.files.length) {
            // バックエンドの期待に合わせて name を調整
            // ここでは同じ name("files") を複数回 append
            Array.from(input.files).forEach(file => fd.append('files', file));
        }

        const xhr = new XMLHttpRequest();
        xhr.open('POST', form.action, true);
        xhr.upload.onprogress = function (evt) {
            if (evt.lengthComputable) {
                const percent = Math.round((evt.loaded / evt.total) * 100);
                bar.style.width = percent + '%';
            }
        };
        xhr.onload = function () {
            submitBtn.disabled = false;
            if (xhr.status >= 200 && xhr.status < 300) {
                resp.classList.remove('error');
                resp.textContent = xhr.responseText || 'アップロードが完了しました。';
                bar.style.width = '100%';
            } else {
                resp.classList.add('error');
                resp.textContent = `エラー: ${xhr.status} ${xhr.statusText}\n` + (xhr.responseText || '');
            }
        };
        xhr.onerror = function () {
            submitBtn.disabled = false;
            resp.classList.add('error');
            resp.textContent = 'ネットワークエラーにより送信に失敗しました。';
        };
        xhr.send(fd);
    });
})();
