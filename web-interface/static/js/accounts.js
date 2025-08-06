// 资金账户管理页面JavaScript
// 全局变量
let currentPage = 1;
let currentPerPage = 20;
let currentSearch = '';

// 页面加载时获取数据
document.addEventListener('DOMContentLoaded', function() {
    loadAccounts();
    loadAccountsSummary();
});

// 加载资金账户数据
function loadAccounts(page = 1, perPage = 20, search = '') {
    currentPage = page;
    currentPerPage = perPage;
    currentSearch = search;

    const url = `/api/accounts?page=${page}&per_page=${perPage}&search=${encodeURIComponent(search)}`;
    
    fetch(url)
        .then(response => response.json())
        .then(data => {
            displayAccounts(data.data);
            updatePagination(data.total, data.page, data.per_page, data.pages);
        })
        .catch(error => {
            console.error('Error:', error);
            document.getElementById('accountsTable').innerHTML = 
                '<tr><td colspan="9" class="text-center text-danger">加载失败，请重试</td></tr>';
        });
}

// 显示资金账户数据
function displayAccounts(accounts) {
    const tbody = document.getElementById('accountsTable');
    
    if (!accounts || accounts.length === 0) {
        tbody.innerHTML = '<tr><td colspan="9" class="text-center text-muted">暂无数据</td></tr>';
        return;
    }

    tbody.innerHTML = accounts.map(account => `
        <tr>
            <td><strong>${account.AccountID}</strong></td>
            <td><span class="badge bg-primary">${account.BrokerID}</span></td>
            <td><span class="text-primary">¥${formatMoney(account.Balance)}</span></td>
            <td><span class="text-success">¥${formatMoney(account.Available)}</span></td>
            <td><span class="text-warning">¥${formatMoney(account.CurrMargin)}</span></td>
            <td><span class="${account.PositionProfit >= 0 ? 'text-success' : 'text-danger'}">
                ¥${formatMoney(account.PositionProfit)}</span></td>
            <td><span class="${account.CloseProfit >= 0 ? 'text-success' : 'text-danger'}">
                ¥${formatMoney(account.CloseProfit)}</span></td>
            <td>${account.TradingDay}</td>
            <td>
                <button class="btn btn-sm btn-outline-info" onclick="showAccountDetail('${account.AccountID}')">
                    <i class="fas fa-eye"></i> 详情
                </button>
            </td>
        </tr>
    `).join('');
}

// 加载资金汇总
function loadAccountsSummary() {
    fetch('/api/accounts/summary')
        .then(response => response.json())
        .then(result => {
            if (!result.success) {
                throw new Error(result.message || '获取汇总数据失败');
            }
            
            const data = result.data;
            document.getElementById('accounts-summary').innerHTML = `
                <div class="row text-center">
                    <div class="col-12 mb-2">
                        <small class="text-muted">账户数量</small><br>
                        <strong class="text-primary">${data.total_accounts || 0}</strong>
                    </div>
                    <div class="col-12 mb-2">
                        <small class="text-muted">总余额</small><br>
                        <strong class="text-success">¥${formatMoney(data.total_balance || 0)}</strong>
                    </div>
                    <div class="col-12 mb-2">
                        <small class="text-muted">可用资金</small><br>
                        <strong class="text-info">¥${formatMoney(data.total_available || 0)}</strong>
                    </div>
                    <div class="col-12 mb-2">
                        <small class="text-muted">占用保证金</small><br>
                        <strong class="text-warning">¥${formatMoney(data.total_margin || 0)}</strong>
                    </div>
                    <div class="col-12">
                        <small class="text-muted">总盈亏</small><br>
                        <strong class="${(data.total_position_profit + data.total_close_profit) >= 0 ? 'text-success' : 'text-danger'}">
                            ¥${formatMoney((data.total_position_profit || 0) + (data.total_close_profit || 0))}
                        </strong>
                    </div>
                </div>
            `;
        })
        .catch(error => {
            console.error('Error loading summary:', error);
            document.getElementById('accounts-summary').innerHTML = 
                '<p class="text-center text-danger">汇总数据加载失败: ' + error.message + '</p>';
        });
}

// 格式化金额
function formatMoney(amount) {
    if (amount === null || amount === undefined) return '0.00';
    return parseFloat(amount).toLocaleString('zh-CN', {
        minimumFractionDigits: 2,
        maximumFractionDigits: 2
    });
}

// 搜索功能
function searchAccounts() {
    const search = document.getElementById('searchInput').value;
    loadAccounts(1, currentPerPage, search);
}

// 回车搜索
document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('searchInput').addEventListener('keypress', function(event) {
        if (event.key === 'Enter') {
            searchAccounts();
        }
    });
});

// 更改每页显示数量
function changePerPage() {
    const perPage = parseInt(document.getElementById('perPageSelect').value);
    loadAccounts(1, perPage, currentSearch);
}

// 刷新数据
function refreshData() {
    loadAccounts(currentPage, currentPerPage, currentSearch);
    loadAccountsSummary();
}

// 显示账户详情
function showAccountDetail(accountId) {
    fetch(`/api/account/${accountId}`)
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                const account = data.data;
                document.getElementById('accountDetailBody').innerHTML = `
                    <div class="row">
                        <div class="col-md-6">
                            <h6>基本信息</h6>
                            <table class="table table-sm">
                                <tr><th>账户ID:</th><td>${account.AccountID}</td></tr>
                                <tr><th>经纪商:</th><td>${account.BrokerID}</td></tr>
                                <tr><th>交易日:</th><td>${account.TradingDay}</td></tr>
                                <tr><th>币种:</th><td>${account.CurrencyID || 'CNY'}</td></tr>
                            </table>
                        </div>
                        <div class="col-md-6">
                            <h6>资金信息</h6>
                            <table class="table table-sm">
                                <tr><th>账户余额:</th><td class="text-primary">¥${formatMoney(account.Balance)}</td></tr>
                                <tr><th>可用资金:</th><td class="text-success">¥${formatMoney(account.Available)}</td></tr>
                                <tr><th>当前保证金:</th><td class="text-warning">¥${formatMoney(account.CurrMargin)}</td></tr>
                                <tr><th>冻结保证金:</th><td>¥${formatMoney(account.FrozenMargin)}</td></tr>
                            </table>
                        </div>
                        <div class="col-md-6">
                            <h6>盈亏信息</h6>
                            <table class="table table-sm">
                                <tr><th>持仓盈亏:</th><td class="${account.PositionProfit >= 0 ? 'text-success' : 'text-danger'}">¥${formatMoney(account.PositionProfit)}</td></tr>
                                <tr><th>平仓盈亏:</th><td class="${account.CloseProfit >= 0 ? 'text-success' : 'text-danger'}">¥${formatMoney(account.CloseProfit)}</td></tr>
                                <tr><th>手续费:</th><td class="text-warning">¥${formatMoney(account.Commission)}</td></tr>
                            </table>
                        </div>
                        <div class="col-md-6">
                            <h6>出入金信息</h6>
                            <table class="table table-sm">
                                <tr><th>入金:</th><td class="text-success">¥${formatMoney(account.Deposit)}</td></tr>
                                <tr><th>出金:</th><td class="text-danger">¥${formatMoney(account.Withdraw)}</td></tr>
                                <tr><th>可取资金:</th><td>¥${formatMoney(account.WithdrawQuota)}</td></tr>
                            </table>
                        </div>
                    </div>
                `;
                new bootstrap.Modal(document.getElementById('accountDetailModal')).show();
            }
        })
        .catch(error => {
            console.error('Error:', error);
            alert('加载账户详情失败');
        });
}

// 分页功能
function updatePagination(total, currentPage, perPage, totalPages) {
    const pagination = document.getElementById('pagination');
    let paginationHTML = '';

    if (totalPages <= 1) {
        pagination.innerHTML = '';
        return;
    }

    // 上一页
    paginationHTML += `
        <li class="page-item ${currentPage === 1 ? 'disabled' : ''}">
            <a class="page-link" href="#" onclick="loadAccounts(${currentPage - 1}, ${perPage}, '${currentSearch}')">上一页</a>
        </li>
    `;

    // 页码
    const startPage = Math.max(1, currentPage - 2);
    const endPage = Math.min(totalPages, currentPage + 2);

    for (let i = startPage; i <= endPage; i++) {
        paginationHTML += `
            <li class="page-item ${i === currentPage ? 'active' : ''}">
                <a class="page-link" href="#" onclick="loadAccounts(${i}, ${perPage}, '${currentSearch}')">${i}</a>
            </li>
        `;
    }

    // 下一页
    paginationHTML += `
        <li class="page-item ${currentPage === totalPages ? 'disabled' : ''}">
            <a class="page-link" href="#" onclick="loadAccounts(${currentPage + 1}, ${perPage}, '${currentSearch}')">下一页</a>
        </li>
    `;

    pagination.innerHTML = paginationHTML;
}

// 导出功能
function exportCurrentAccounts() {
    const url = `/api/export/accounts?search=${encodeURIComponent(currentSearch)}`;
    window.location.href = url;
}

function exportAllAccounts() {
    const url = '/api/export/accounts';
    window.location.href = url;
} 