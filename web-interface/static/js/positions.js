// 全局变量
let currentPositionPage = 1;
let currentPositionSearch = '';
let currentInvestorFilter = '';
let currentBrokerFilter = '';
let totalPositionPages = 1;

// 页面加载完成后初始化
document.addEventListener('DOMContentLoaded', function() {
    loadPositions();
    loadPositionStats();
    
    // 搜索框回车事件
    document.getElementById('searchInput').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') {
            searchPositions();
        }
    });
    
    // 筛选框回车事件
    document.getElementById('filterInvestorID').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') {
            applyFilters();
        }
    });
    
    document.getElementById('filterBrokerID').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') {
            applyFilters();
        }
    });
});

// 加载持仓数据
async function loadPositions(page = 1, search = '', investorId = '', brokerId = '') {
    showPositionLoading();
    currentPositionPage = page;
    currentPositionSearch = search;
    currentInvestorFilter = investorId;
    currentBrokerFilter = brokerId;
    
    try {
        const params = new URLSearchParams({
            page: page,
            per_page: 20,
            search: search,
            investor_id: investorId,
            broker_id: brokerId
        });
        
        const response = await fetch(`/api/positions?${params}`);
        const data = await response.json();
        
        if (data.data && data.data.length > 0) {
            displayPositions(data.data);
            updatePositionPagination(data);
            document.getElementById('totalPositionCount').textContent = data.total;
            showPositionTable();
        } else {
            showNoPositionData();
        }
    } catch (error) {
        console.error('加载持仓数据失败:', error);
        showToast('加载持仓数据失败，请稍后重试', 'error');
        showNoPositionData();
    }
}

// 显示持仓数据
function displayPositions(positions) {
    const tbody = document.getElementById('positionsTable');
    tbody.innerHTML = '';
    
    positions.forEach(position => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td><strong>${position.InstrumentID || '-'}</strong></td>
            <td>${position.InvestorID || '-'}</td>
            <td><span class="badge bg-info">${position.BrokerID || '-'}</span></td>
            <td>${getPositionDirectionText(position.PosiDirection)}</td>
            <td>${getHedgeFlagText(position.HedgeFlag)}</td>
            <td>${position.YdPosition || 0}</td>
            <td><strong>${position.Position || 0}</strong></td>
            <td class="${position.PositionProfit >= 0 ? 'text-success' : 'text-danger'}">
                ${parseFloat(position.PositionProfit || 0).toFixed(2)}
            </td>
            <td>${parseFloat(position.UseMargin || 0).toFixed(2)}</td>
            <td>${position.TradingDay || '-'}</td>
            <td>
                <button class="btn btn-outline-info btn-sm" onclick="showPositionDetail('${position.id || position.InstrumentID}')">
                    <i class="fas fa-eye"></i> 详情
                </button>
            </td>
        `;
        tbody.appendChild(row);
    });
}

// 更新分页
function updatePositionPagination(data) {
    totalPositionPages = data.pages;
    const pagination = document.getElementById('positionPagination');
    pagination.innerHTML = '';
    
    // 上一页
    const prevItem = document.createElement('li');
    prevItem.className = `page-item ${currentPositionPage === 1 ? 'disabled' : ''}`;
    prevItem.innerHTML = `
        <a class="page-link" href="#" onclick="changePositionPage(${currentPositionPage - 1})">
            <i class="fas fa-chevron-left"></i>
        </a>
    `;
    pagination.appendChild(prevItem);
    
    // 页码
    const startPage = Math.max(1, currentPositionPage - 2);
    const endPage = Math.min(totalPositionPages, currentPositionPage + 2);
    
    if (startPage > 1) {
        const firstItem = document.createElement('li');
        firstItem.className = 'page-item';
        firstItem.innerHTML = `<a class="page-link" href="#" onclick="changePositionPage(1)">1</a>`;
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
        pageItem.className = `page-item ${i === currentPositionPage ? 'active' : ''}`;
        pageItem.innerHTML = `<a class="page-link" href="#" onclick="changePositionPage(${i})">${i}</a>`;
        pagination.appendChild(pageItem);
    }
    
    if (endPage < totalPositionPages) {
        if (endPage < totalPositionPages - 1) {
            const ellipsis = document.createElement('li');
            ellipsis.className = 'page-item disabled';
            ellipsis.innerHTML = '<span class="page-link">...</span>';
            pagination.appendChild(ellipsis);
        }
        
        const lastItem = document.createElement('li');
        lastItem.className = 'page-item';
        lastItem.innerHTML = `<a class="page-link" href="#" onclick="changePositionPage(${totalPositionPages})">${totalPositionPages}</a>`;
        pagination.appendChild(lastItem);
    }
    
    // 下一页
    const nextItem = document.createElement('li');
    nextItem.className = `page-item ${currentPositionPage === totalPositionPages ? 'disabled' : ''}`;
    nextItem.innerHTML = `
        <a class="page-link" href="#" onclick="changePositionPage(${currentPositionPage + 1})">
            <i class="fas fa-chevron-right"></i>
        </a>
    `;
    pagination.appendChild(nextItem);
}

// 切换页面
function changePositionPage(page) {
    if (page >= 1 && page <= totalPositionPages && page !== currentPositionPage) {
        loadPositions(page, currentPositionSearch, currentInvestorFilter, currentBrokerFilter);
    }
}

// 搜索持仓
function searchPositions() {
    const searchValue = document.getElementById('searchInput').value.trim();
    loadPositions(1, searchValue, currentInvestorFilter, currentBrokerFilter);
}

// 应用筛选条件
function applyFilters() {
    const investorId = document.getElementById('filterInvestorID').value.trim();
    const brokerId = document.getElementById('filterBrokerID').value.trim();
    loadPositions(1, currentPositionSearch, investorId, brokerId);
}

// 清除筛选条件
function clearFilters() {
    document.getElementById('filterInvestorID').value = '';
    document.getElementById('filterBrokerID').value = '';
    document.getElementById('searchInput').value = '';
    loadPositions(1, '', '', '');
}

// 刷新持仓数据
function refreshPositions() {
    loadPositions(currentPositionPage, currentPositionSearch, currentInvestorFilter, currentBrokerFilter);
    loadPositionStats();
    showToast('持仓数据已刷新', 'success');
}

// 显示持仓详情
async function showPositionDetail(positionId) {
    const modal = new bootstrap.Modal(document.getElementById('positionModal'));
    const detailsContainer = document.getElementById('positionDetails');
    
    // 显示加载状态
    detailsContainer.innerHTML = `
        <div class="text-center">
            <div class="spinner-border text-primary" role="status">
                <span class="visually-hidden">加载中...</span>
            </div>
            <div class="mt-2">正在加载持仓详情...</div>
        </div>
    `;
    
    modal.show();
    
    try {
        const response = await fetch(`/api/position/${positionId}`);
        const result = await response.json();
        
        if (result.success) {
            const position = result.data;
            detailsContainer.innerHTML = `
                <table class="table table-bordered detail-table">
                    <tr><th>持仓ID</th><td>${position.id || '-'}</td></tr>
                    <tr><th>合约代码</th><td>${position.InstrumentID || '-'}</td></tr>
                    <tr><th>投资者账号</th><td>${position.InvestorID || '-'}</td></tr>
                    <tr><th>经纪商代码</th><td>${position.BrokerID || '-'}</td></tr>
                    <tr><th>持仓方向</th><td>${getPositionDirectionText(position.PosiDirection)}</td></tr>
                    <tr><th>投机套保标志</th><td>${getHedgeFlagText(position.HedgeFlag)}</td></tr>
                    <tr><th>持仓日期</th><td>${getPositionDateText(position.PositionDate)}</td></tr>
                    <tr><th>上日持仓</th><td>${position.YdPosition || 0}</td></tr>
                    <tr><th>今日持仓</th><td>${position.Position || 0}</td></tr>
                    <tr><th>多头冻结</th><td>${position.LongFrozen || 0}</td></tr>
                    <tr><th>空头冻结</th><td>${position.ShortFrozen || 0}</td></tr>
                    <tr><th>持仓成本</th><td>${parseFloat(position.PositionCost || 0).toFixed(4)}</td></tr>
                    <tr><th>持仓盈亏</th><td class="${position.PositionProfit >= 0 ? 'text-success' : 'text-danger'}">
                        ${parseFloat(position.PositionProfit || 0).toFixed(2)}
                    </td></tr>
                    <tr><th>占用保证金</th><td>${parseFloat(position.UseMargin || 0).toFixed(2)}</td></tr>
                    <tr><th>开仓成本</th><td>${parseFloat(position.OpenCost || 0).toFixed(4)}</td></tr>
                    <tr><th>手续费</th><td>${parseFloat(position.Commission || 0).toFixed(2)}</td></tr>
                    <tr><th>平仓盈亏</th><td class="${position.CloseProfit >= 0 ? 'text-success' : 'text-danger'}">
                        ${parseFloat(position.CloseProfit || 0).toFixed(2)}
                    </td></tr>
                    <tr><th>交易日</th><td>${position.TradingDay || '-'}</td></tr>
                    <tr><th>结算编号</th><td>${position.SettlementID || '-'}</td></tr>
                    <tr><th>交易所代码</th><td>${position.ExchangeID || '-'}</td></tr>
                </table>
            `;
        } else {
            detailsContainer.innerHTML = `
                <div class="alert alert-warning">
                    <i class="fas fa-exclamation-triangle"></i>
                    ${result.message || '获取持仓详情失败'}
                </div>
            `;
        }
    } catch (error) {
        console.error('获取持仓详情失败:', error);
        detailsContainer.innerHTML = `
            <div class="alert alert-danger">
                <i class="fas fa-exclamation-circle"></i>
                获取持仓详情失败，请稍后重试
            </div>
        `;
    }
}

// 加载投资者列表
async function loadInvestors() {
    const modal = new bootstrap.Modal(document.getElementById('investorsModal'));
    const listContainer = document.getElementById('investorsList');
    
    // 显示加载状态
    listContainer.innerHTML = `
        <div class="text-center">
            <div class="spinner-border text-primary" role="status">
                <span class="visually-hidden">加载中...</span>
            </div>
            <div class="mt-2">正在加载投资者列表...</div>
        </div>
    `;
    
    modal.show();
    
    try {
        const response = await fetch('/api/investors');
        const result = await response.json();
        
        if (result.data && result.data.length > 0) {
            let tableHtml = `
                <div class="table-responsive">
                    <table class="table table-striped">
                        <thead>
                            <tr>
                                <th>投资者账号</th>
                                <th>经纪商</th>
                                <th>持仓数量</th>
                                <th>总持仓</th>
                                <th>总盈亏</th>
                                <th>操作</th>
                            </tr>
                        </thead>
                        <tbody>
            `;
            
            result.data.forEach(investor => {
                tableHtml += `
                    <tr>
                        <td><strong>${investor.InvestorID}</strong></td>
                        <td><span class="badge bg-info">${investor.BrokerID}</span></td>
                        <td>${investor.position_count}</td>
                        <td>${investor.total_position}</td>
                        <td class="${investor.total_profit >= 0 ? 'text-success' : 'text-danger'}">
                            ${parseFloat(investor.total_profit || 0).toFixed(2)}
                        </td>
                        <td>
                            <button class="btn btn-sm btn-primary" onclick="filterByInvestor('${investor.InvestorID}')">
                                查看持仓
                            </button>
                        </td>
                    </tr>
                `;
            });
            
            tableHtml += `
                        </tbody>
                    </table>
                </div>
            `;
            
            listContainer.innerHTML = tableHtml;
        } else {
            listContainer.innerHTML = `
                <div class="alert alert-info">
                    <i class="fas fa-info-circle"></i>
                    暂无投资者数据
                </div>
            `;
        }
    } catch (error) {
        console.error('加载投资者列表失败:', error);
        listContainer.innerHTML = `
            <div class="alert alert-danger">
                <i class="fas fa-exclamation-circle"></i>
                加载投资者列表失败，请稍后重试
            </div>
        `;
    }
}

// 按投资者筛选
function filterByInvestor(investorId) {
    // 关闭模态框
    const modal = bootstrap.Modal.getInstance(document.getElementById('investorsModal'));
    modal.hide();
    
    // 设置筛选条件
    document.getElementById('filterInvestorID').value = investorId;
    applyFilters();
}

// 加载持仓统计信息
async function loadPositionStats() {
    try {
        const response = await fetch('/api/position_stats');
        const result = await response.json();
        
        if (!result.success) {
            throw new Error(result.message || '获取统计数据失败');
        }
        
        const stats = result.data;
        const statsContainer = document.getElementById('position-stats');
        
        statsContainer.innerHTML = `
            <div class="row g-2 mb-3">
                <div class="col-6">
                    <div class="card bg-primary text-white">
                        <div class="card-body text-center p-2">
                            <div class="h4 mb-0">${stats.total_positions || 0}</div>
                            <small>总持仓数</small>
                        </div>
                    </div>
                </div>
                <div class="col-6">
                    <div class="card bg-success text-white">
                        <div class="card-body text-center p-2">
                            <div class="h4 mb-0">${stats.total_investors || 0}</div>
                            <small>投资者数</small>
                        </div>
                    </div>
                </div>
            </div>
            <div class="row g-2 mb-3">
                <div class="col-6">
                    <div class="card bg-info text-white">
                        <div class="card-body text-center p-2">
                            <div class="h4 mb-0">${stats.total_instruments || 0}</div>
                            <small>持仓品种</small>
                        </div>
                    </div>
                </div>
                <div class="col-6">
                    <div class="card bg-warning text-white">
                        <div class="card-body text-center p-2">
                            <div class="h4 mb-0">${stats.total_position || 0}</div>
                            <small>总手数</small>
                        </div>
                    </div>
                </div>
            </div>
            <div class="mb-3">
                <div class="row g-2">
                    <div class="col-12">
                        <div class="card ${stats.total_position_profit >= 0 ? 'bg-success' : 'bg-danger'} text-white">
                            <div class="card-body text-center p-2">
                                <div class="h5 mb-0">
                                    ${stats.total_position_profit >= 0 ? '+' : ''}${parseFloat(stats.total_position_profit || 0).toLocaleString('zh-CN', {minimumFractionDigits: 2})}
                                </div>
                                <small>总持仓盈亏 (元)</small>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
            <div class="mb-2">
                <div class="row g-2">
                    <div class="col-12">
                        <div class="card bg-secondary text-white">
                            <div class="card-body text-center p-2">
                                <div class="h5 mb-0">
                                    ${parseFloat(stats.total_margin || 0).toLocaleString('zh-CN', {minimumFractionDigits: 2})}
                                </div>
                                <small>总保证金 (元)</small>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        `;
    } catch (error) {
        console.error('加载持仓统计信息失败:', error);
        document.getElementById('position-stats').innerHTML = `
            <div class="alert alert-warning alert-sm">
                <i class="fas fa-exclamation-triangle"></i>
                统计信息加载失败: ${error.message}
            </div>
        `;
    }
}

// 导出功能
function exportCurrentPositions() {
    const params = new URLSearchParams({
        search: currentPositionSearch,
        investor_id: currentInvestorFilter,
        broker_id: currentBrokerFilter
    });
    window.open(`/api/export/positions/csv?${params}`, '_blank');
    showToast('正在导出当前搜索结果...', 'info');
}

function exportAllPositions() {
    window.open('/api/export/positions/csv', '_blank');
    showToast('正在导出全部持仓数据...', 'info');
}

function exportSpecificPositions() {
    const positions = document.getElementById('exportPositionIDs').value.trim();
    if (!positions) {
        showToast('请输入要导出的持仓ID', 'warning');
        return;
    }
    
    window.open(`/api/export/positions/csv?positions=${encodeURIComponent(positions)}`, '_blank');
    showToast('正在导出指定持仓数据...', 'info');
}

// 显示/隐藏状态
function showPositionLoading() {
    document.getElementById('positionLoadingIndicator').style.display = 'block';
    document.getElementById('positionTableContainer').style.display = 'none';
    document.getElementById('noPositionDataMessage').style.display = 'none';
}

function showPositionTable() {
    document.getElementById('positionLoadingIndicator').style.display = 'none';
    document.getElementById('positionTableContainer').style.display = 'block';
    document.getElementById('noPositionDataMessage').style.display = 'none';
}

function showNoPositionData() {
    document.getElementById('positionLoadingIndicator').style.display = 'none';
    document.getElementById('positionTableContainer').style.display = 'none';
    document.getElementById('noPositionDataMessage').style.display = 'block';
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
function getPositionDirectionText(direction) {
    const directions = {
        '1': '<span class="badge bg-danger">空头</span>',
        '2': '<span class="badge bg-success">多头</span>',
        '3': '<span class="badge bg-warning">净头寸</span>'
    };
    return directions[direction] || `<span class="badge bg-secondary">${direction || '-'}</span>`;
}

function getHedgeFlagText(flag) {
    const flags = {
        '1': '<span class="badge bg-primary">投机</span>',
        '2': '<span class="badge bg-info">套利</span>',
        '3': '<span class="badge bg-warning">套保</span>',
        '5': '<span class="badge bg-secondary">做市商</span>'
    };
    return flags[flag] || `<span class="badge bg-light text-dark">${flag || '-'}</span>`;
}

function getPositionDateText(date) {
    const dates = {
        '1': '今日仓',
        '2': '历史仓'
    };
    return dates[date] || (date || '-');
} 