import numpy as np
from matplotlib import pyplot as plt
import os
from pylab import mpl

mpl.rcParams['font.sans-serif'] = ['FangSong']  # 指定默认字体
mpl.rcParams['axes.unicode_minus'] = False  # 解决保存图像是负号'-'显示为方块的问题


#---------------------------------------------------
#---------------------------------------------------
#---------------------------------------------------
#--------OMP算法实现---------------------------------
#---------------------------------------------------
#---------------------------------------------------
#---------------------------------------------------
#---------------------------------------------------
#---------------------------------------------------

#-- OMP算法辅助函数
def find_max_atom_vector(rk_1, A):
    '''
        #-- 从字典A中找出最大原子向量
        #-- 输入参数：
        #-- rk_1: 前一个残差向量
        #-- A：字典
        #-- 输出：返回最大原子向量下标
    '''
    return np.argmax([np.abs(np.dot(ai,rk_1))  for i,ai in enumerate(A.T[:,]) ])



# -- OMP算法求解 b = Ax 稀疏度最小的x
def OMP(A, b, e, k):
    '''
        # -- OMP算法求解 b = Ax 稀疏度最小的x
        # -- 输入参数： 
        # -- A:  字典
        # -- b:  需要稀疏表达的向量
        # -- e:  停止条件1，当误差小于e时可以直接停止迭代
        # -- k:  停止条件2，最大迭代次数
        # -- 输出：b在A上的稀疏表达x
    '''
    N = A.shape[1]
    x = np.zeros([N])  #初始化稀疏表达
    S = []  # 支撑集
    rk = b  # 初始残差
    indexs = []  #选择的基下标


    #  迭代
    for i in range(k):
        # 选择残差投影最大的原子向量
        index = find_max_atom_vector(rk, A,)

        # 选择的下标，即最终的x不为0的位置
        indexs.append(index)

        # 加入新选择的基
        S.append(A[:, index])

        # 利用矩阵的逆重新计算表达式
        Ak = np.array(S).T
        xk = np.linalg.inv(Ak.T.dot(Ak)).dot(Ak.T.dot(b))
      
        # 更新残差
        rk = b - Ak.dot(xk)

        # 残差已经足够小了
        if(np.sqrt(np.mean(np.square(rk))) - e < 0):
            break

    # 构造最终求得的x并返回
    x[indexs] = xk.squeeze()
    return x


    m = 20
    n = 10000
    b = np.random.normal(size = (m, 1))
    A = np.random.normal(size = (m, n))

    x = OMP(A,b.copy(),1e-5,7)
    plt.plot(b, 'g', label='原始数据')
    plt.plot(A.dot(x), 'r', label='重构数据')

    plt.xlabel('indexs')
    plt.ylabel('values')
    plt.title('OMP效果图')
    plt.legend()

    plt.show()




#  测试OMP算法
def test_OMP():
    m = 20
    n = 10000
    b = np.random.normal(size=(m, 1))
    A = np.random.normal(size=(m, n))

    #  拟合
    x = OMP(A,b.copy(),1e-5,7)
    plt.plot(b, 'g', label='原始数据')
    plt.plot(A.dot(x), 'r', label='重构数据')

    plt.xlabel('indexs')
    plt.ylabel('values')
    plt.title('OMP效果图')
    plt.legend()

    plt.show()

    #  分析
    errors = []

    for i in range(20):
        x = OMP(A, b.copy(), 1e-5, i+1)
        new_x = A.dot(x)
        error = np.sqrt(np.sum(np.square(b.squeeze()-new_x)))
        errors.append(error)

    plt.plot([i+1 for i in range(20)], errors, 'g', label='误差')

    plt.xlim((0,25))
    plt.xlabel('稀疏度')
    plt.ylabel('误差值')
    plt.title('OMP效果图')
    plt.legend()   
    plt.show()



#--------------------------------------------------------------
#--------------------------------------------------------------
#--------K-SVD算法实现------------------------------------------
#--------------------------------------------------------------
#--------------------------------------------------------------
#--------------------------------------------------------------
#--------------------------------------------------------------

#-- 矩阵的F范式---
def normalization_F(A):
    '''
        计算矩阵的F范式，定义见Lecture6
        输入参数：矩阵A
        输出：矩阵A的F范式
    '''
    return np.sqrt(np.sum(np.square(A)))


#-- K-SVD 求解
#-- 本次K-SVD算法没有使用F范式来退出迭代,而是迭代完给定的L次数
def K_SVD(D, S, Y, e, L ):
    '''
        利用K-SVD算法训练字典D

        输入参数：
            原始字典D：m x K 固定大小
            训练数据Y：m x N
            稀疏度S：OMP算法退出条件
            残差e：OMP算法退出条件
            迭代次数L：K-SVD迭代次数
            目标：yi = Dxi + ei
                || xi ||0 <= S <= m; || ei ||(2,2) <= e;(任意1 <= i <= N) 
        输出：
            D：训练好的字典
            X：数据Y在字典D上的系稀疏表达，大小为 K x N
    '''
    m = D.shape[0]
    K = D.shape[1]
    N = Y.shape[1]
    print('Y = DX, 其中 Y = %d x %d ; D = %d x %d ; X = %d x %d'%(m,N,m,K,K,N))

    X = np.zeros([K, N])
    for i in range(L):
        # 步骤一： 利用OMP寻找最相关稀疏矩阵X
        for i in range(N):
            X[:,i] = OMP(D,Y[:,i],e,S)

        # 步骤二：字典更新
        # 每次更新一列，固定其他列不变
        for k in range(K):
            gk_T = X[k] # 1 x N

            # 通过索引将gk_T,Ek收缩,不包含0
            I = [i for i,res in enumerate(gk_T) if res]

            # 全为0直接跳过不进行处理
            if len(I) == 0:
                continue

            #-- 求解收缩后的Ek
            D[:,k] = 0
            Ek_shrink = (Y - np.dot(D,X))[:,I]

            #-- 通过奇异值分解收缩后的Ek,得到新的dk和收缩后的gk_T
            #-- 奇异值分解,Sigma为奇异值向量，需要使用np.diag使其成为矩阵
            #-- 分解后的V需要进行转置再取第一列更新gk_T，Ek_shrink = U*Sigma*V_T
            U, Sigma, V = np.linalg.svd(Ek_shrink, full_matrices=False)

            #-- 用新的dk,gk_T 更新字典
            D[:,k] = U[:,0]
            X[k,I] = Sigma[0]*(V[0,:])
    return D, X

 
#-----------------------------------------------------------------
#-----------------------------------------------------------------
#-----------------------------------------------------------------
#----------数据集处理----------------------------------------------
#-----------------------------------------------------------------
#-----------------------------------------------------------------
#-----------------------------------------------------------------
#-----------------------------------------------------------------

#-- 数据集目录
root_dir = r'DataSet'

#-- 用38张图片中的22张构建训练数据（8*8）
def save_train():
    files = os.listdir(root_dir)
    train_data = []
    width = 168
    height = 192
    for k in range(22):
        file = os.path.join(root_dir, files[k])
        img = plt.imread(file)
        for i in range(height//8):
            for j in range(width//8):
                train_data.append(img[i*8:i*8+8, j*8:j*8+8])

    #-- 将生成的训练数据用numpy保存
    train_data = np.array(train_data)
    np.save('train_data',train_data)


#-- 用38张图片中的后16张构建测试数据（8*8）
def save_test():
    files = os.listdir(root_dir)
    test_data = []
    width = 168
    height = 192
    for k in range(16):
        file = os.path.join(root_dir, files[k+22])
        img = plt.imread(file)
        for i in range(height//8):
            for j in range(width//8):
                test_data.append(img[i*8:i*8+8, j*8:j*8+8])

    #-- 将生成的训练数据用numpy保存
    test_data = np.array(test_data)
    np.save('test_data',test_data)


#-- 读取22张训练集,number为读取的数量
def read_train(number:int=22):
    data = np.load('train_data.npy')
    return data[:number*504]

#-- 读取测试数据并进行不同程度模糊处理,number为读取的图片数量
#-- ratio为模糊个数
def read_test(ratio:int,number:int=16):
    if number > 22:
        print("测试数据不足!")
        return None

    data =  np.load('test_data.npy')
    data = data[:504*number]
    data = data.reshape(-1,64)

    # 每一行中随机选取 ratio个像素置0
    if not ratio:
        return data.reshape(-1,8,8)
    indexs = np.arange(64)
    for row in range(data.shape[0]):
        random_index = np.random.permutation(indexs)[:ratio]
        data[row][random_index] = 0
    return data.reshape(-1,8,8)



#-- 将处理后的数据转换为图像数据并显示
#-- test_data: -1 x 64
def reverse(test_data):
    test_data = test_data.reshape(-1,8,8)
    width = 168
    height = 192
    imgs = []
    for k in range(test_data.shape[0] // 504):
        img = np.zeros((height,width))
        for i in range(height//8):
            for j in range(width//8):
                img[i*8:i*8+8, j*8:j*8+8] = test_data[k*504 + i*21 + j]
        imgs.append(img)
    
    for i in range(len(imgs)):
        plt.subplot(5,5,i+1)
        plt.imshow(imgs[i], cmap='gray')
 
    plt.show()

# 显示原始训练集和测试集
def show():
    train = read_train().reshape(-1,64)
    reverse(train)
    test = read_test(0).reshape(-1,64)
    reverse(test)
    test = read_test(int(0.5*64)).reshape(-1,64)
    reverse(test)
    test = read_test(int(0.7*64)).reshape(-1,64)
    reverse(test)


#------------------------------------------------------------
#------------------------------------------------------------
#------------------------------------------------------------
#------------------------------------------------------------
#----------训练与测试-----------------------------------------
#------------------------------------------------------------
#------------------------------------------------------------
#------------------------------------------------------------
#------------------------------------------------------------


#-- 对矩阵进行列归一化操作
#-- 使得列向量为单位向量
def normalization(A):
    for j in range(A.shape[1]):
        res_sum = np.sqrt(np.sum(np.square(A[:,j])))
        A[:,j] = np.divide(A[:,j],res_sum)
    return A

#-- 训练字典
#-- Y = 64 x 11088
#-- D = 64 x 441
#-- 稀疏编码 OMP
def train():
    S = 10  #  稀疏度
    L = 3   #  迭代次数
    e = 0.00001
    K = 441
    Y = read_train(22).reshape(-1,64).T

    #  对训练数据进行归一化处理
    Y_norm = np.zeros(Y.shape)

    #  每一列归一化
    for i in range(Y.shape[1]):
        normlize = np.linalg.norm(Y[:,i])
        mean = np.sum(Y[:,i])/Y.shape[0]
        Y_norm[:,i] = (Y[:,i]-mean)/normlize

    # -- 随机生成初始字典D0
    indexs = np.arange(Y.shape[1])[:K]
    D0 = Y_norm[:,indexs]
    
    # -- 归一化为单位向量
    D0 = normalization(D0)

    # 训练字典
    D, X = K_SVD(D0.copy(),S,Y_norm,e,L)

    #  保存结果
    np.save('D_train',D)
    np.save('X_train',X)





#-- 利用学习的字典和稀疏矩阵对测试集进行重构
def test():
    D_file = 'D_train.npy'
    X_file = 'X_train.npy'

    D = np.load(D_file)
    X = np.load(X_file)

    # 获取模糊后的测试数据
    ratio = int(0.5 * 64)
    test_data = read_test(ratio) # -1 x 8 x 8
    test_data = test_data.reshape(-1,64).T # 64 x -1
    
    #  重构后的数据
    new_data = np.zeros(test_data.shape)

    # 归一化处理,只针对非零数据,然后使用OMP算法获取稀疏表达，计算后加入重构数据
    for i in range(test_data.shape[1]):
        index = np.nonzero(test_data[:,i])[0]
        if not index.shape[0]:
            continue
        normlize = np.linalg.norm(test_data[:,i][index])
        mean = np.sum(test_data[:,i][index])/index.shape[0]
        data_norm = (test_data[:,i][index]-mean)/normlize
        x = OMP(D[index,:],data_norm,0.0001,10)

        #  重构后需要反归一化处理
        new_data[:,i] = D.dot(x)*normlize + mean

    #  显示重构图像
    reverse(new_data.T)

    #  打印平均误差
    print(RMSE(read_test(0).reshape(-1,64).T,new_data))



#  计算原始数据和重构数据的误差
#  数据大小为 64 x -1
def RMSE(origin_data, constructed_data):
    error = 0
    for i in range(origin_data.shape[1]):
        error += np.sqrt(np.sum(np.square(origin_data[:,i]-constructed_data[:,i]))/64)
    
    return error / origin_data.shape[1] / 16




#---------------------------------------------------------
#---------------------------------------------------------
#---------------------------------------------------------
#---------------------------------------------------------
#------------主函数测试------------------------------------
#---------------------------------------------------------
#---------------------------------------------------------
#---------------------------------------------------------
#---------------------------------------------------------

if __name__ == '__main__':
    #  测试OMP算法
    test_OMP()

    #  显示训练集图片和测试集图片包括残差处理后的图片
    show()

    # #  训练字典并保存
    # train()

    #  对残差图片进行恢复处理并计算平均误差
    test()
