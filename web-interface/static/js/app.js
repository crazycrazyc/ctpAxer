// 全局变量
let currentPage = 1;
let currentSearch = '';
let totalPages = 1;

// 页面加载完成后初始化
document.addEventListener('DOMContentLoaded', function() {
    loadInstruments();
    loadStats();
    
    // 搜索框回车事件
    document.getElementById('searchInput').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') {
            searchInstruments();
        }
    });
});

// 加载合约数据
async function loadInstruments(page = 1, search = '') {
    showLoading();
    currentPage = page;
    currentSearch = search;
    
    try {
        const response = await fetch(`/api/instruments?page=${page}&per_page=20&search=${encodeURIComponent(search)}`);
        const data = await response.json();
        
        if (data.data && data.data.length > 0) {
            displayInstruments(data.data);
            updatePagination(data);
            document.getElementById('totalCount').textContent = data.total;
            showTable();
        } else {
            showNoData();
        }
    } catch (error) {
        console.error('加载数据失败:', error);
        showToast('加载数据失败，请稍后重试', 'error');
        showNoData();
    }
}

// 显示合约数据
function displayInstruments(instruments) {
    const tbody = document.getElementById('instrumentsTable');
    tbody.innerHTML = '';
    
    instruments.forEach(instrument => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td><strong>${instrument.InstrumentID || '-'}</strong></td>
            <td>${instrument.InstrumentName || '-'}</td>
            <td><span class="badge bg-primary">${instrument.ExchangeID || '-'}</span></td>
            <td>${instrument.ProductID || '-'}</td>
            <td>${instrument.VolumeMultiple || '-'}</td>
            <td>${instrument.PriceTick || '-'}</td>
            <td>${instrument.IsTrading == 1 ? 
                '<span class="status-active"><i class="fas fa-check-circle"></i> 交易中</span>' : 
                '<span class="status-inactive"><i class="fas fa-times-circle"></i> 停止交易</span>'
            }</td>
            <td>${instrument.ExpireDate || '-'}</td>
            <td>
                <button class="btn btn-outline-info btn-sm" onclick="showInstrumentDetail('${instrument.InstrumentID}')">
                    <i class="fas fa-eye"></i> 详情
                </button>
            </td>
        `;
        tbody.appendChild(row);
    });
}

// 更新分页
function updatePagination(data) {
    totalPages = data.pages;
    const pagination = document.getElementById('pagination');
    pagination.innerHTML = '';
    
    // 上一页
    const prevItem = document.createElement('li');
    prevItem.className = `page-item ${currentPage === 1 ? 'disabled' : ''}`;
    prevItem.innerHTML = `
        <a class="page-link" href="#" onclick="changePage(${currentPage - 1})">
            <i class="fas fa-chevron-left"></i>
        </a>
    `;
    pagination.appendChild(prevItem);
    
    // 页码
    const startPage = Math.max(1, currentPage - 2);
    const endPage = Math.min(totalPages, currentPage + 2);
    
    if (startPage > 1) {
        const firstItem = document.createElement('li');
        firstItem.className = 'page-item';
        firstItem.innerHTML = `<a class="page-link" href="#" onclick="changePage(1)">1</a>`;
        pagination.appendChild(firstItem);
        
        if (startPage > 2) {
            const ellipsis = document.createElement('li');
            ellipsis.className = 'page-item disabled';
            ellipsis.innerHTML = '<span class="page-link">...</span>';
            pagination.appendChild(ellipsis);
        }
    }
    
    for (let i = startPage; i <= endPage; i++) {
        const pageItem = document.createElement('li');
        pageItem.className = `page-item ${i === currentPage ? 'active' : ''}`;
        pageItem.innerHTML = `<a class="page-link" href="#" onclick="changePage(${i})">${i}</a>`;
        pagination.appendChild(pageItem);
    }
    
    if (endPage < totalPages) {
        if (endPage < totalPages - 1) {
            const ellipsis = document.createElement('li');
            ellipsis.className = 'page-item disabled';
            ellipsis.innerHTML = '<span class="page-link">...</span>';
            pagination.appendChild(ellipsis);
        }
        
        const lastItem = document.createElement('li');
        lastItem.className = 'page-item';
        lastItem.innerHTML = `<a class="page-link" href="#" onclick="changePage(${totalPages})">${totalPages}</a>`;
        pagination.appendChild(lastItem);
    }
    
    // 下一页
    const nextItem = document.createElement('li');
    nextItem.className = `page-item ${currentPage === totalPages ? 'disabled' : ''}`;
    nextItem.innerHTML = `
        <a class="page-link" href="#" onclick="changePage(${currentPage + 1})">
            <i class="fas fa-chevron-right"></i>
        </a>
    `;
    pagination.appendChild(nextItem);
}

// 切换页面
function changePage(page) {
    if (page >= 1 && page <= totalPages && page !== currentPage) {
        loadInstruments(page, currentSearch);
    }
}

// 搜索合约
function searchInstruments() {
    const searchValue = document.getElementById('searchInput').value.trim();
    loadInstruments(1, searchValue);
}

// 刷新数据
function refreshData() {
    loadInstruments(currentPage, currentSearch);
    loadStats();
    showToast('数据已刷新', 'success');
}

// 显示合约详情
async function showInstrumentDetail(instrumentId) {
    const modal = new bootstrap.Modal(document.getElementById('instrumentModal'));
    const detailsContainer = document.getElementById('instrumentDetails');
    
    // 显示加载状态
    detailsContainer.innerHTML = `
        <div class="text-center">
            <div class="spinner-border text-primary" role="status">
                <span class="visually-hidden">加载中...</span>
            </div>
            <div class="mt-2">正在加载合约详情...</div>
        </div>
    `;
    
    modal.show();
    
    try {
        const response = await fetch(`/api/instrument/${instrumentId}`);
        const result = await response.json();
        
        if (result.success) {
            const instrument = result.data;
            detailsContainer.innerHTML = `
                <table class="table table-bordered detail-table">
                    <tr><th>合约代码</th><td>${instrument.InstrumentID || '-'}</td></tr>
                    <tr><th>合约名称</th><td>${instrument.InstrumentName || '-'}</td></tr>
                    <tr><th>交易所代码</th><td>${instrument.ExchangeID || '-'}</td></tr>
                    <tr><th>交易所合约代码</th><td>${instrument.ExchangeInstID || '-'}</td></tr>
                    <tr><th>品种代码</th><td>${instrument.ProductID || '-'}</td></tr>
                    <tr><th>品种类型</th><td>${getProductClassName(instrument.ProductClass) || '-'}</td></tr>
                    <tr><th>交割年份</th><td>${instrument.DeliveryYear || '-'}</td></tr>
                    <tr><th>交割月份</th><td>${instrument.DeliveryMonth || '-'}</td></tr>
                    <tr><th>合约乘数</th><td>${instrument.VolumeMultiple || '-'}</td></tr>
                    <tr><th>最小变动价位</th><td>${instrument.PriceTick || '-'}</td></tr>
                    <tr><th>创建日期</th><td>${instrument.CreateDate || '-'}</td></tr>
                    <tr><th>上市日期</th><td>${instrument.OpenDate || '-'}</td></tr>
                    <tr><th>到期日期</th><td>${instrument.ExpireDate || '-'}</td></tr>
                    <tr><th>开始交割日</th><td>${instrument.StartDelivDate || '-'}</td></tr>
                    <tr><th>结束交割日</th><td>${instrument.EndDelivDate || '-'}</td></tr>
                    <tr><th>合约生命周期状态</th><td>${getLifePhaseText(instrument.InstLifePhase) || '-'}</td></tr>
                    <tr><th>是否交易</th><td>${instrument.IsTrading == 1 ? 
                        '<span class="status-active"><i class="fas fa-check-circle"></i> 是</span>' : 
                        '<span class="status-inactive"><i class="fas fa-times-circle"></i> 否</span>'
                    }</td></tr>
                    <tr><th>持仓类型</th><td>${getPositionTypeText(instrument.PositionType) || '-'}</td></tr>
                    <tr><th>持仓日期类型</th><td>${getPositionDateTypeText(instrument.PositionDateType) || '-'}</td></tr>
                    <tr><th>多头保证金率</th><td>${instrument.LongMarginRatio || '-'}</td></tr>
                    <tr><th>空头保证金率</th><td>${instrument.ShortMarginRatio || '-'}</td></tr>
                </table>
            `;
        } else {
            detailsContainer.innerHTML = `
                <div class="alert alert-warning">
                    <i class="fas fa-exclamation-triangle"></i>
                    ${result.message || '获取合约详情失败'}
                </div>
            `;
        }
    } catch (error) {
        console.error('获取合约详情失败:', error);
        detailsContainer.innerHTML = `
            <div class="alert alert-danger">
                <i class="fas fa-exclamation-circle"></i>
                获取合约详情失败，请稍后重试
            </div>
        `;
    }
}

// 加载统计信息
async function loadStats() {
    const statsContainer = document.getElementById('stats-info');
    
    try {
        const response = await fetch('/api/stats');
        const result = await response.json();
        
        // 检查响应状态
        if (!response.ok) {
            throw new Error(result.message || '服务器响应错误');
        }
        
        const stats = result.data || result; // 兼容新旧格式
        
        // 显示警告信息（如果有部分查询失败）
        let warningHtml = '';
        if (result.status === 'partial_success') {
            warningHtml = `
                <div class="alert alert-warning alert-sm mb-2">
                    <i class="fas fa-exclamation-triangle"></i>
                    ${result.message}
                </div>
            `;
        }
        
        statsContainer.innerHTML = warningHtml + `
            <div class="row g-2">
                <div class="col-6">
                    <div class="card stats-card text-white">
                        <div class="card-body text-center">
                            <div class="stats-number">${stats.total_instruments?.[0]?.count || 0}</div>
                            <div class="stats-label">总合约数</div>
                        </div>
                    </div>
                </div>
                <div class="col-6">
                    <div class="card stats-card text-white">
                        <div class="card-body text-center">
                            <div class="stats-number">${stats.active_instruments?.[0]?.count || 0}</div>
                            <div class="stats-label">交易中</div>
                        </div>
                    </div>
                </div>
            </div>
            <div class="mt-3">
                <h6><i class="fas fa-building"></i> 交易所分布</h6>
                ${(stats.exchanges || []).slice(0, 5).map(ex => 
                    `<div class="d-flex justify-content-between">
                        <span>${ex.ExchangeID}</span>
                        <span class="badge bg-secondary">${ex.count}</span>
                    </div>`
                ).join('') || '<div class="text-muted small">暂无数据</div>'}
            </div>
        `;
        
        // 如果成功加载，显示一个成功的提示（可选）
        if (result.status === 'success') {
            console.log('统计信息加载成功');
        }
        
    } catch (error) {
        console.error('加载统计信息失败:', error);
        statsContainer.innerHTML = `
            <div class="alert alert-danger alert-sm">
                <i class="fas fa-exclamation-circle"></i>
                <div>统计信息加载失败</div>
                <small class="text-muted">${error.message}</small>
                <div class="mt-2">
                    <button class="btn btn-sm btn-outline-light" onclick="loadStats()">
                        <i class="fas fa-redo"></i> 重试
                    </button>
                </div>
            </div>
        `;
    }
}

// 导出功能
function exportCurrentView() {
    const searchParam = currentSearch ? `&search=${encodeURIComponent(currentSearch)}` : '';
    window.open(`/api/export/csv?${searchParam.substring(1)}`, '_blank');
    showToast('正在导出当前搜索结果...', 'info');
}

function exportAll() {
    window.open('/api/export/csv', '_blank');
    showToast('正在导出全部合约数据...', 'info');
}

function exportSpecificInstruments() {
    const instruments = document.getElementById('exportInstruments').value.trim();
    if (!instruments) {
        showToast('请输入要导出的合约代码', 'warning');
        return;
    }
    
    window.open(`/api/export/csv?instruments=${encodeURIComponent(instruments)}`, '_blank');
    showToast('正在导出指定合约数据...', 'info');
}

// 显示/隐藏状态
function showLoading() {
    document.getElementById('loadingIndicator').style.display = 'block';
    document.getElementById('tableContainer').style.display = 'none';
    document.getElementById('noDataMessage').style.display = 'none';
}

function showTable() {
    document.getElementById('loadingIndicator').style.display = 'none';
    document.getElementById('tableContainer').style.display = 'block';
    document.getElementById('noDataMessage').style.display = 'none';
}

function showNoData() {
    document.getElementById('loadingIndicator').style.display = 'none';
    document.getElementById('tableContainer').style.display = 'none';
    document.getElementById('noDataMessage').style.display = 'block';
}

// 显示Toast通知
function showToast(message, type = 'info') {
    const toast = document.getElementById('toast');
    const toastBody = document.getElementById('toastBody');
    
    // 设置图标和样式
    const icons = {
        success: 'fas fa-check-circle text-success',
        error: 'fas fa-exclamation-circle text-danger',
        warning: 'fas fa-exclamation-triangle text-warning',
        info: 'fas fa-info-circle text-primary'
    };
    
    const icon = icons[type] || icons.info;
    toast.querySelector('.toast-header i').className = icon;
    toastBody.textContent = message;
    
    const bsToast = new bootstrap.Toast(toast);
    bsToast.show();
}

// 辅助函数
function getProductClassName(productClass) {
    const classes = {
        '1': '期货',
        '2': '期权',
        '3': '组合'
    };
    return classes[productClass] || productClass;
}

function getLifePhaseText(phase) {
    const phases = {
        '0': '未上市',
        '1': '上市',
        '2': '停牌',
        '3': '到期'
    };
    return phases[phase] || phase;
}

function getPositionTypeText(type) {
    const types = {
        '1': '净持仓',
        '2': '综合持仓'
    };
    return types[type] || type;
}

function getPositionDateTypeText(type) {
    const types = {
        '1': '使用历史持仓',
        '2': '不使用历史持仓'
    };
    return types[type] || type;
} 