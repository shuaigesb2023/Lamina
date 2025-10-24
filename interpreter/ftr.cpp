#include<vector>
#include<utility>
#include<string>
//TODO:与其说是模板类，实际上已经不是了，还缺少完善、优化、规范
namespace templatestdftr {
    /*
    将函数模板化
    类型需要支持 + - / % * = == 
    */
    template <class TP = unsigned long long>
    TP unllgcd(TP a, TP b) {//求最大公约数，默认a>=b
        if (b == TP(0))return a;
        return unllgcd(b, a % b);
    }
    std::pair<unsigned long long, unsigned long long> jiance(std::vector<short>& n) {
        //检测循环部分，返回循环的开始和结束（左闭右闭）
        unsigned long long h1[10] = {};//使用哈希表记录数字出现的次数
        unsigned long long h2[10] = {};

        for (short& i : n)h1[i]++;//第一轮遍历记录每个数字的总出现次数（使用迭代器）
        bool flag = true;

        for (unsigned long long i = 0; i < n.size(); i++) {//第二轮遍历
            flag = true;
            for (int j = 0; j < 10; j++) {
                if ((h1[j] - h2[j]) & 1) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                unsigned long long i1 = i + 1, i2 = n.size();
                unsigned long long ti1, ti2;
                bool ok = true;
                while (!((i2 - i1) & 1) and ok) {
                    //检测是否循环
                    //使用双指针
                    //这段改的比较多不写小注释了
                    //原理为不断将循环范围减半直到不循环为止如果第一轮不循环侧进入第二轮遍历
                    ti1 = i1;
                    ti2 = i1 + ((i2 - i1) >> 1);
                    while (ti2 < i2) {
                        if (n[ti1] != n[ti2]) {
                            ok = false;
                            break;
                        }
                        ti1++;
                        ti2++;
                    }if (ok) {
                        i2 = i1 + ((i2 - i1) >> 1);
                    }
                    else if (i2 == n.size()) {
                        break;
                    }
                    else {
                        return { i1,i2 - 1 };
                    }
                }if (ok) {//如果保持循环返回
                    return { i1,i2 - 1 };
                }
            }
            h2[n[i]]++;
        }
        return { n.size()-1,n.size()-1};

    }
    template <class TP = unsigned long long>
    std::pair<TP, TP> float_to_rational(const std::string num) {
        //返回值第一个为分子第二个为分母
        TP up = TP(0), down = TP(1);//up分子，down分母
        unsigned long long i = 0;
        while (i < num.size() and num[i] != '.' and num[i] != 'e' and num[i] != 'E') {//处理整数部分
            up = up * TP(10) + TP(num[i] - '0');
            i++;
        }
        if (i == num.size() or num[i] == 'e' or num[i] == 'E') {
            if (i != num.size() and (num[i] == 'e' or num[i] == 'E')) {//如果为科学计数法
                i++;
                if (num[i] == '-') {
                    i++;
                    TP p = TP(0);
                    while (i < num.size()) {
                        p = p * TP(10) + TP(num[i] - '0');
                        i++;
                    }
                    down = down * (TP(10).power(p));
                }
                else {
                    TP p = TP(0);
                    while (i < num.size()) {
                        p = p * TP(10) + TP(num[i] - '0');
                        i++;
                    }
                    up = up * (TP(10).power(p));
                }

            }
            return{ up,down };//不是小数，直接返回
        }
        i++;
        std::vector<short> n;//使用vector容器暂时存储小数位
        while (i < num.size() and num[i] != '.' and num[i] != 'e' and num[i] != 'E') {
            n.emplace_back(num[i] - '0');
            i++;
        }
        if (i == num.size() or ((num[i] == 'e' or num[i] == 'E'))) {//检查是否为循环小数
            //处理不循环小数
            //直接添加到末尾
            for (short& i : n) {
                up = up * TP(10) + TP(i);
                down = down * TP(10);
            }
        }
        else {
            //处理循环小数
            //先检测循环部分，再将不循环部分直接添加，再加（循环部分/999...9（循环部分长度个9）/pow（10，len（不循环部分）））
            std::pair<unsigned long long, unsigned long long> xun = jiance(n);

            //用变量临时记录循环部分
            TP xup = TP(0), xdown = TP(0);
            for (unsigned long long i = xun.first; i <= xun.second; i++) {
                xup = xup * TP(10) + TP(n[i]);
                xdown = xdown * TP(10) + TP(9);
            }

            //将不循环部分添加
            for (unsigned long long i = 0; i < xun.first; i++) {
                up = up * TP(10) + TP(n[i]);
                down = down * TP(10);
            }

            //通分
            up = up * xdown;
            down = down * xdown;

            //相加
            up = up + xup;
        }

        if (i != num.size() and (num[i] == 'e' or num[i] == 'E')) {//如果为科学计数法
            i++;
            if (num[i] == '-') {
                i++;
                TP p = TP(0);
                while (i < num.size()) {
                    p = p * TP(10) + TP(num[i] - '0');
                    i++;
                }
                down = down * (TP(10).power(p));
            }
            else {
                TP p = TP(0);
                while (i < num.size()) {
                    p = p * TP(10) + TP(num[i] - '0');
                    i++;
                }
                up = up * (TP(10).power(p));
            }
            
        }

        //约分化简，使用gcd
        TP g = unllgcd<TP>(up, down);
        up = up / g;
        down = down / g;

        //返回
        return { up,down };
    }
}