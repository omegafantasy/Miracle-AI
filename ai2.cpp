#include "ai_client.h"
#include "gameunit.h"
#include "card.hpp"
#include "calculator.h"

#include <fstream>
#include <stdlib.h>
#include <random>
#include <time.h>

using namespace std;
using namespace gameunit;
using namespace calculator;
using namespace card;

class AI : public AiClient
{
private:
    Pos miracle_pos;
public:
    //选择初始卡组
    void chooseCards(); //(根据初始阵营)选择初始卡组

    void play(); //玩家需要编写的ai操作函数

    void start();
        void start_attack();
            int findbest_swordsman(vector<Unit> target_list,vector<Unit> ally_list,Unit a);
            int findbest_priest(vector<Unit> target_list,vector<Unit> ally_list,Unit a);
            int findbest_archer(vector<Unit> target_list,vector<Unit> ally_list,Unit a);
            int findbest_inferno(vector<Unit> target_list,vector<Unit> ally_list,Unit a);

        void start_move();
            Pos start_sword(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list);
            Pos start_priest(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list);
            Pos start_archer(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list);
            int getdamage(Pos pos,string s);
            int getalldamage(Pos pos);
            int getdamageex(Pos pos,string s);
            int getdamageex2(Pos pos,string s1,string s2);
            bool can_sword(Unit a,Pos pos,vector<Unit> enemy_list);
            bool can_archer(Unit a,Pos pos,vector<Unit> enemy_list);
            bool can_priest(Unit a,Pos pos,vector<Unit> enemy_list);
            bool att(Pos pos,vector<Unit> enemy_list,Unit a);
            int sort_sword(Pos pos);
            int sort_archer(Pos pos);
            int sort_priest(Pos pos);
            int sort_inferno(Pos pos);
            vector<int> center_judge(string s);

        void start_summon();

    void mid();
        void mid_attack();

        void mid_move();
            Pos mid_sword(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list);
            Pos mid_priest(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list);
             Pos mid_inferno(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list);
             Pos mid_archer(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list);
        void mid_artifact();
            bool use_artifact(Unit a);
            bool use_artifact();
        void mid_summon();

    void last1();
        void last1_move();
            Pos last1_priest(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list);
};

void AI::chooseCards()
{
    // (根据初始阵营)选择初始卡组

    /*
     * artifacts和creatures可以修改
     * 【进阶】在选择卡牌时，就已经知道了自己的所在阵营和先后手，因此可以在此处根据先后手的不同设置不同的卡组和神器
     */
    my_artifacts = {"InfernoFlame"};
    my_creatures = {"Archer", "Swordsman", "Priest"};
    init();
}

int camp1;//0为先手，1为后手
int camp2;
int K;//具体坐标根据己方阵营调整

Pos enemy_center,center,side,side1,side2,side3,side4,ll1,ll2,ll3,ll4,l1,l2,l3,l4,c1,c2,c3,c4,r1,r2,r3,r4,rr1,rr2,rr3,rr4
,lll1,lll2,lll3,rrr1,rrr2,rrr3;//阵型标准位置


bool cmp_ally(const Unit &unit1, const Unit &unit2){
    //return1则选unit1，return0则选unit2
        if (unit1.can_atk != unit2.can_atk)  //首先要能动
            return unit2.can_atk < unit1.can_atk;
        else if (unit1.type != unit2.type) {//剑士>弓>牧师
            auto type_id_gen = [](const string &type_name) {
                if (type_name == "Swordsman") return 0;
                else if (type_name == "Archer") return 1;
                else return 2;
            };
            return (type_id_gen(unit1.type) < type_id_gen(unit2.type));
        } 
        else{//离阵营近的优先攻击。
            return cube_distance(unit1.pos,center)<cube_distance(unit2.pos,center);
        }
}

int AI::findbest_swordsman(vector<Unit> target_list,vector<Unit> ally_list,Unit a){
    int ret=-1;
    int level=100;
    int distance=10;
    for (auto enemy : target_list) {
        int dis = cube_distance(enemy.pos,center);
        if (enemy.atk_range[0] <= dis && dis <= enemy.atk_range[1])//如果能打到神迹一定要打
            return enemy.id;
        if(canAttack(a,enemy)&&a.atk>=enemy.hp&&enemy.holy_shield==0){
            //如果能秒则打
            level=1;
            ret=enemy.id;
        }
        for(auto ally:ally_list){//如果能打自己人则先打
            if(canAttack(enemy,ally)&&level>2){
                level=2;
                ret=enemy.id;
                break;
            }
        }
        if(canAttack(a,enemy)&&enemy.type=="archer"&&level>3){
            level=3;
            ret=enemy.id;
        }
        if(canAttack(a,enemy)&&enemy.type=="priest"&&level>4){
            level=4;
            ret=enemy.id;
        }
        if(canAttack(a,enemy)&&level>4&&dis<distance){
            level=5;
            distance=dis;
            ret=enemy.id;
        }
    }
    return ret;
}

int AI::findbest_priest(vector<Unit> target_list,vector<Unit> ally_list,Unit a){
    int ret=-1;
    int level=100;
    int distance=10;
    for (auto enemy : target_list) {
        int dis = cube_distance(enemy.pos,center);
        if (enemy.atk_range[0] <= dis && dis <= enemy.atk_range[1])//如果能打到神迹一定要打
            return enemy.id;
        if(canAttack(a,enemy)&&enemy.type=="BlackBat")
            return enemy.id;
        if(canAttack(enemy,a)&&enemy.atk>=a.hp)//如果会被反杀则不打
            continue;
        if(canAttack(a,enemy)&&a.atk>=enemy.hp&&enemy.holy_shield==0){
            //如果能秒则打
            level=1;
            ret=enemy.id;
        }
        for(auto ally:ally_list){//如果能打自己人则先打
            if(canAttack(enemy,ally)&&level>2){
                level=2;
                ret=enemy.id;
                break;
            }
        }
        if(canAttack(a,enemy)&&level>2&&dis<distance){
            distance=dis;
            ret=enemy.id;
            level=5;
        }
    }
    return ret;
}

int AI::findbest_archer(vector<Unit> target_list,vector<Unit> ally_list,Unit a){
    int ret=-1;
    int level=100;
    int distance=10;
    for (auto enemy : target_list) {
        int dis = cube_distance(enemy.pos,center);
        if (enemy.atk_range[0] <= dis && dis <= enemy.atk_range[1])//如果能打到神迹一定要打
            return enemy.id;
        if(canAttack(enemy,a)&&enemy.atk>=a.hp)//如果会被反杀则不打
            continue;
        if(canAttack(a,enemy)&&a.atk>=enemy.hp&&enemy.holy_shield==0){
            //如果能秒则打
            level=1;
            ret=enemy.id;
        }
        for(auto ally:ally_list){//如果能打自己人则先打
            if(canAttack(enemy,ally)&&level>2){
                level=2;
                ret=enemy.id;
                break;
            }
        }
        if(canAttack(a,enemy)&&dis<distance&&level>2){
            distance=dis;
            level=5;
            ret=enemy.id;
        }
    }
    if(ret==-1){
        for(auto enemy:target_list){
            if(canAttack(a,enemy)&&a.atk>=enemy.hp&&enemy.holy_shield==0){//弓箭手互秒
                return enemy.id;
            }
        }
    }
    return ret;
}
int AI::findbest_inferno(vector<Unit> target_list,vector<Unit> ally_list,Unit a){
    int ret=-1;
    int level=100;
    int distance=10;
    for (auto enemy : target_list) {
        int dis = cube_distance(enemy.pos,center);
        if (enemy.atk_range[0] <= dis && dis <= enemy.atk_range[1])//如果能打到神迹一定要打
            return enemy.id;
        if(canAttack(a,enemy)&&a.atk>=enemy.hp&&enemy.holy_shield==0){
            //如果能秒则打
            level=1;
            ret=enemy.id;
        }
        for(auto ally:ally_list){//如果能打自己人则先打
            if(canAttack(enemy,ally)&&level>2){
                level=2;
                ret=enemy.id;
                break;
            }
        }
        if(canAttack(a,enemy)&&enemy.type=="archer"&&level>3){
            level=3;
            ret=enemy.id;
        }
        if(canAttack(a,enemy)&&enemy.type=="priest"&&level>4){
            level=4;
            ret=enemy.id;
        }
        if(canAttack(a,enemy)&&level>4&&dis<distance){
            level=5;
            distance=dis;
            ret=enemy.id;
        }
    }
    return ret;
}
void AI::start_attack(){
    //处理生物的战斗

    auto ally_list = getUnitsByCamp(camp1);
    auto enemy_list=getUnitsByCamp(camp2);

    //按顺序排列好单位，依次攻击
    sort(ally_list.begin(), ally_list.end(), cmp_ally);

    for (const auto &ally:ally_list) {
        if (!ally.can_atk) break;
        vector<Unit> target_list;
        for (const auto &enemy : enemy_list)
            if (AiClient::canAttack(ally, enemy))
                target_list.push_back(enemy);
        if (target_list.empty()) continue;

        if (ally.type == "Priest") {
            int j=findbest_priest(target_list,ally_list,ally);
            if(j!=-1)
                attack(ally.id,j);
        } else if (ally.type == "Swordsman") {
            int j=findbest_swordsman(target_list,ally_list,ally);
            if(j!=-1)
                attack(ally.id,j);
        } else if (ally.type == "Archer") {
            int j=findbest_archer(target_list,ally_list,ally);
            if(j!=-1)
                attack(ally.id,j);
        }
    }
}

int AI::getdamage(Pos pos,string s){
    auto enemy_list=getUnitsByCamp(camp2);
    int ret=0;
    for(auto enemy:enemy_list){
        if(enemy.type==s){
            int dis=cube_distance(enemy.pos,pos);
            if(enemy.atk_range[0] <= dis && dis <= enemy.atk_range[1])
                ret+=enemy.atk;
        }
    }
    return ret;
}
int AI::getalldamage(Pos pos){
    auto enemy_list=getUnitsByCamp(camp2);
    int ret=0;
    for(auto enemy:enemy_list){
        int dis=cube_distance(enemy.pos,pos);
        if(enemy.atk_range[0] <= dis && dis <= enemy.atk_range[1])
            ret+=enemy.atk;
    }
    return ret;
}
int AI::getdamageex(Pos pos,string s){
    auto enemy_list=getUnitsByCamp(camp2);
    int ret=0;
    for(auto enemy:enemy_list){
        if(enemy.type!=s){
            int dis=cube_distance(enemy.pos,pos);
            if(enemy.atk_range[0] <= dis && dis <= enemy.atk_range[1])
                ret+=enemy.atk;
        }
    }
    return ret;
}
int AI::getdamageex2(Pos pos,string s1,string s2){
    auto enemy_list=getUnitsByCamp(camp2);
    int ret=0;
    for(auto enemy:enemy_list){
        if(enemy.type!=s1&&enemy.type!=s2){
            int dis=cube_distance(enemy.pos,pos);
            if(enemy.atk_range[0] <= dis && dis <= enemy.atk_range[1])
                ret+=enemy.atk;
        }
    }
    return ret;
}


bool AI::can_sword(Unit a,Pos pos,vector<Unit> enemy_list){
    if(getUnitByPos(pos,0).id!=-1&&pos!=a.pos)
        return 0;
    if(getdamage(pos,"Archer")+getdamage(pos,"BlackBat")>=a.hp)
        return 0;
    return 1;
}

bool AI::can_priest(Unit a,Pos pos,vector<Unit> enemy_list){
    if(getUnitByPos(pos,0).id!=-1&&pos!=a.pos)
        return 0;
    if(getdamageex(pos,"BlackBat")>=a.hp)
        return 0;
    return 1;
}

bool AI::can_archer(Unit a,Pos pos,vector<Unit> enemy_list){
    if(getUnitByPos(pos,0).id!=-1&&pos!=a.pos)
        return 0;
    if(getdamageex(pos,"Archer")>=a.hp)
        return 0;
    return 1;
}

bool AI::att(Pos pos,vector<Unit> enemy_list,Unit a){
    if(getUnitByPos(pos,0).id!=-1)
        return 0;
    for(auto enemy:enemy_list){
        int dis=cube_distance(enemy.pos,pos);
        if(a.atk_range[0] <= dis && dis <= a.atk_range[1])
            return 1;
    }
    return 0;
}

vector<int> AI::center_judge(string s){
    auto all_pos=all_pos_in_map();
    vector<Pos> left,right;
    for(auto pos:all_pos){
        if(K==1){
            if(get<2>(pos)>=0&&cube_distance(pos,c2)<=3){
                left.push_back(pos);
            }
            if(get<2>(pos)<=0&&cube_distance(pos,c2)<=3){
                right.push_back(pos);
            }
        }
        else{
            if(get<2>(pos)<=0&&cube_distance(pos,c2)<=3){
                left.push_back(pos);
            }
            if(get<2>(pos)>=0&&cube_distance(pos,c2)<=3){
                right.push_back(pos);
            }
        }
    }
    int l=0;
    int r=0;
    for(auto pos:left){
        auto a=getUnitByPos(pos,0);
        if(a.camp==camp1&&a.type==s){
            l++;
        }
    }
    for(auto pos:right){
        auto a=getUnitByPos(pos,0);
        if(a.camp==camp1&&a.type==s){
            r++;
        }
    }
    return {l,r};
}

int AI::sort_sword(Pos pos){
    auto enemy_list=getUnitsByCamp(camp2);
    int ret=10;
    for(auto enemy:enemy_list){
        int dis=cube_distance(enemy.pos,pos);
        if(dis==1&&enemy.type=="Inferno")
            ret+=20;
        else if(dis==1&&enemy.type=="Archer")
            ret+=5;
        else if(dis==1&&enemy.type=="Priest")
            ret+=2;
        else if(dis==1)
            ret+=1;
    }
    if(cube_distance(pos,c1)<=3)
        ret+=2;
    if(pos==l1||pos==r1||pos==ll1||pos==lll1||pos==rr1||pos==rrr1)
        ret-=4;
    ret-=getalldamage(pos);
    return ret;
}
int AI::sort_priest(Pos pos){
    auto enemy_list=getUnitsByCamp(camp2);
    int ret=10;
    if(pos==l2||pos==l3)
        ret+=10;
    
    ret-=getalldamage(pos);
    return ret;
}
int AI::sort_archer(Pos pos){
    auto enemy_list=getUnitsByCamp(camp2);
    int ret=10;
    for(auto enemy:enemy_list){
        int dis=cube_distance(enemy.pos,pos);
        if(dis>=3&&dis<=4)
            ret+=2;
    }
    if(cube_distance(pos,c1)==3)
        ret+=2;
    ret-=getalldamage(pos);
    return ret;
}
int AI::sort_inferno(Pos pos){
    auto enemy_list=getUnitsByCamp(camp2);
    int ret=50;
    for(auto enemy:enemy_list){
        int dis=cube_distance(enemy.pos,pos);
        if(dis==1&&enemy.type=="Inferno")
            ret+=100;
        else if(dis==1)
            ret+=2;
    }
    ret-=getalldamage(pos);
    ret-=cube_distance(pos,enemy_center)*2;
    if(cube_distance(pos,enemy_center)==1)
        ret+=1000;
    return ret;
}

Pos AI::start_sword(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list){
    Pos ret=a.pos;
    reach_pos_list.push_back(a.pos);
    int level=20;
    int val1=-100;
    int val2=-100;
    int val3=-100;
    for(auto pos:reach_pos_list){
        auto result=center_judge("Swordsman");
        if(result[0]<2&&result[1]>=2&&K==1){
            if(get<2>(pos)>=0&&cube_distance(pos,c2)<=4){
                level=0;
                int now=sort_sword(pos);
                if(now>val3){
                    val3=now;
                    ret=pos;
                };
            }
        }
        else if(result[0]<2&&result[1]>=2&&K==-1){
            if(get<2>(pos)<=0&&cube_distance(pos,c2)<=4){
                level=0;
                int now=sort_sword(pos);
                if(now>val3){
                    val3=now;
                    ret=pos;
                };
            }
        }
        else if(result[0]>=2&&result[1]<2&&K==-1){
            if(get<2>(pos)>=0&&cube_distance(pos,c2)<=4){
                level=0;
                int now=sort_sword(pos);
                if(now>val3){
                    val3=now;
                    ret=pos;
                };
            }
        }
        else if(result[0]>=2&&result[1]<2&&K==1){
            if(get<2>(pos)<=0&&cube_distance(pos,c2)<=4){
                level=0;
                int now=sort_sword(pos);
                if(now>val3){
                    val3=now;
                    ret=pos;
                };
            }
        }

        if(pos==c3&&can_sword(a,c3,enemy_list)&&level>1){
            ret=c3;
            level=1;
            continue;
        }
        if(pos==rr3&&can_sword(a,rr3,enemy_list)&&level>2){
            ret=rr3;
            level=2;
            continue;
        }
        if(pos==side&&can_sword(a,side,enemy_list)&&level>3){
            ret=side;
            level=3;
            continue;
        }
        if(can_sword(a,pos,enemy_list)&&level>3){
            level=4;
            int now=sort_sword(pos);
            if(now>val1){
                val1=now;
                ret=pos;
            }
            continue;
        }
        if(level>4){
            level=5;
            int now=sort_sword(pos);
            if(now>val2){
                val2=now;
                ret=pos;
            }
        }
    }
    return ret;
}
Pos AI::start_priest(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list){
    Pos ret=a.pos;
    reach_pos_list.push_back(a.pos);
    int level=10;
    int val1=0;
    int val2=0;
    int val3=-100;
    for(auto pos:reach_pos_list){
        auto result=center_judge("Priest");
        if(result[0]==0&&result[1]>=2&&K==1){
            if(get<2>(pos)>=0&&cube_distance(pos,c2)<=4){
                level=0;
                int now=sort_priest(pos);
                if(now>val3){
                    val3=now;
                    ret=pos;
                };
            }
        }
        else if(result[0]==0&&result[1]>=2&&K==-1){
            if(get<2>(pos)<=0&&cube_distance(pos,c2)<=4){
                level=0;
                int now=sort_priest(pos);
                if(now>val3){
                    val3=now;
                    ret=pos;
                };
            }
        }
        else if(result[0]>=2&&result[1]==0&&K==-1){
            if(get<2>(pos)>=0&&cube_distance(pos,c2)<=4){
                level=0;
                int now=sort_priest(pos);
                if(now>val3){
                    val3=now;
                    ret=pos;
                };
            }
        }
        else if(result[0]>=2&&result[1]==0&&K==1){
            if(get<2>(pos)<=0&&cube_distance(pos,c2)<=4){
                level=0;
                int now=sort_priest(pos);
                if(now>val3){
                    val3=now;
                    ret=pos;
                };
            }
        }
        if(pos==r3&&can_priest(a,r3,enemy_list)&&level>1){
            ret=r3;
            level=1;
            continue;
        }
        if(pos==c2&&can_priest(a,c2,enemy_list)&&level>2){
            ret=c2;
            level=2;
            continue;
        }
        if(pos==r2&&can_priest(a,r2,enemy_list)&&level>3){
            ret=r2;
            level=3;
            continue;
        }
        if(pos==c3&&can_priest(a,c3,enemy_list)&&level>4){
            ret=c3;
            level=4;
            continue;
        }
        if(pos==rr3&&can_priest(a,rr3,enemy_list)&&level>5){
            ret=rr3;
            level=5;
            continue;
        }
        if(can_priest(a,pos,enemy_list)&&level>5){
            level=6;
            int now=sort_priest(pos);
            if(now>val1){
                val1=now;
                ret=pos;
            }
            continue;
        }
        if(level>6){
            level=7;
            int now=sort_priest(pos);
            if(now>val2){
                val2=now;
                ret=pos;
            }
        }
    }
    return ret;
}
Pos AI::start_archer(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list){
    Pos ret=a.pos;
    reach_pos_list.push_back(a.pos);
    int level=10;
    int val1=0;
    int val2=0;
    int val3=-100;
    for(auto pos:reach_pos_list){
        auto result=center_judge("Archer");
        if(result[0]==0&&result[1]>=2&&K==1){
            if(get<2>(pos)>=0&&cube_distance(pos,c2)<=4){
                level=0;
                int now=sort_archer(pos);
                if(now>val3){
                    val3=now;
                    ret=pos;
                };
            }
        }
        else if(result[0]==0&&result[1]>=2&&K==-1){
            if(get<2>(pos)<=0&&cube_distance(pos,c2)<=4){
                level=0;
                int now=sort_archer(pos);
                if(now>val3){
                    val3=now;
                    ret=pos;
                };
            }
        }
        else if(result[0]>=2&&result[1]==0&&K==-1){
            if(get<2>(pos)>=0&&cube_distance(pos,c2)<=4){
                level=0;
                int now=sort_archer(pos);
                if(now>val3){
                    val3=now;
                    ret=pos;
                };
            }
        }
        else if(result[0]>=2&&result[1]==0&&K==1){
            if(get<2>(pos)<=0&&cube_distance(pos,c2)<=4){
                level=0;
                int now=sort_archer(pos);
                if(now>val3){
                    val3=now;
                    ret=pos;
                };
            }
        }
        if(pos==c4&&can_archer(a,c4,enemy_list)&&level>1){
            ret=c4;
            level=1;
            continue;
        }
        if(pos==rrr3&&can_archer(a,rrr3,enemy_list)&&level>2){
            ret=rrr3;
            level=2;
            continue;
        }
        if(pos==c1&&can_archer(a,c1,enemy_list)&&level>3){
            ret=c1;
            level=3;
            continue;
        }
        if(can_archer(a,pos,enemy_list)&&level>3){
            level=4;
            int now=sort_archer(pos);
            if(now>val1){
                val1=now;
                ret=pos;
            }
            continue;
        }
        if(level>4){
            level=5;
            int now=sort_archer(pos);
            if(now>val2){
                val2=now;
                ret=pos;
            }
        }
    }
    return ret;
}
Pos AI::mid_inferno(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list){
    Pos ret=a.pos;
    reach_pos_list.push_back(a.pos);
    int val=-100;
    for(auto pos:reach_pos_list){
        int now=sort_inferno(pos);
        if(now>val){
            val=now;
            ret=pos;
        }
    }
    return ret;
}

Pos AI::mid_archer(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list){
    Pos ret=a.pos;
    reach_pos_list.push_back(a.pos);
    int level=10;
    for(auto pos:reach_pos_list){
        if(pos==rr4&&can_archer(a,rr4,enemy_list)){
            ret=rr4;
            level=1;
            continue;
        }
        if(pos==ll4&&can_archer(a,ll4,enemy_list)&&level>2){
            ret=ll4;
            level=2;
            continue;
        }
        if(pos==c1&&can_archer(a,c1,enemy_list)&&level>3){
            ret=c1;
            level=3;
            continue;
        }
        if(pos==c2&&can_archer(a,c2,enemy_list)&&level>4){
            ret=c2;
            level=4;
            continue;
        }
        if(pos==c4&&can_archer(a,c4,enemy_list)&&level>5){
            ret=c4;
            level=5;
            continue;
        }

        if(can_archer(a,pos,enemy_list)&&level>6&&att(pos,enemy_list,a)&&cube_distance(center,pos)>=3&&cube_distance(center,pos)<=5){
            ret=pos;
            level=6;
            continue;
            
        }
        if(can_archer(a,pos,enemy_list)&&level>7&&att(pos,enemy_list,a)){
            ret=pos;
            level=7;
            continue;
            
        }
        if(can_archer(a,pos,enemy_list)&&level>8&&cube_distance(center,pos)>=3&&cube_distance(center,pos)<=5){
            ret=pos;
            level=8;
            continue;
            
        }
        if(can_archer(a,pos,enemy_list)&&level>9){
            ret=pos;
            level=9;
            continue;
        }
    }
    return ret;
}

void AI::start_move(){
    //处理生物的移动

    /*
     * 先动所有剑士，尽可能向敌方神迹移动
     * 若目标驻扎点上没有地面单位，则让弓箭手向目标驻扎点移动，否则尽可能向敌方神迹移动
     * 然后若目标驻扎点上没有地面单位，则让火山之龙向目标驻扎点移动，否则尽可能向敌方神迹移动
     * 【进阶】一味向敌方神迹移动并不一定是个好主意
     * 在移动的时候可以考虑一下避开敌方生物攻击范围实现、为己方强力生物让路、堵住敌方出兵点等策略
     * 如果采用其他生物组合，可以考虑抢占更多驻扎点
     */
    auto ally_list = getUnitsByCamp(my_camp);
    sort(ally_list.begin(), ally_list.end(), [](const Unit &_unit1, const Unit &_unit2) {
        auto type_id_gen = [](const string &type_name) {
            if (type_name == "Swordsman") return 0;
            else if (type_name == "Archer") return 1;
            else return 2;
        };
        return type_id_gen(_unit1.type) < type_id_gen(_unit2.type);
    });
    for (const auto &ally : ally_list) {
        if (!ally.can_move) continue;
        auto reach_pos_with_dis = reachable(ally, map);
            vector<Pos> reach_pos_list;
            for (const auto &reach_pos : reach_pos_with_dis) {
                for (auto pos : reach_pos)
                    reach_pos_list.push_back(pos);
            }
            if (reach_pos_list.empty()) continue;
        auto enemy_list=getUnitsByCamp(camp2);

        if (ally.type == "Swordsman") {
            Pos ps=start_sword(ally,reach_pos_list,enemy_list);
            if(ps==ally.pos)
                continue;
            move(ally.id, ps);
        } 
        else if(ally.type=="Priest"){
           Pos ps=start_priest(ally,reach_pos_list,enemy_list);
            if(ps==ally.pos)
                continue;
            move(ally.id, ps);
        }
        else if(ally.type=="Archer"){
            Pos ps=start_archer(ally,reach_pos_list,enemy_list);
            if(ps==ally.pos)
                continue;
            move(ally.id, ps);
        }
    }
}


void AI::start_summon(){
    //最后进行召唤
        //将所有本方出兵点按照到对方基地的距离排序，从近到远出兵
    bool suc=true;
    while(suc){
        suc=false;
        vector<Pos> summon_pos_list = getSummonPosByCamp(camp1);
        sort(summon_pos_list.begin(), summon_pos_list.end(), [this](Pos _pos1, Pos _pos2) {
            return cube_distance(_pos1,center) < cube_distance(_pos2,center);
        });
        vector<Pos> available_summon_pos_list;
        for (auto pos:summon_pos_list) {
            auto unit_on_pos_ground = getUnitByPos(pos, false);
            if (unit_on_pos_ground.id == -1) available_summon_pos_list.push_back(pos);
        }
        if(available_summon_pos_list.empty())return;

        //统计各个生物的可用数量，在假设出兵点无限的情况下，按照1个剑士、1个弓箭手、1个火山龙的顺序召唤
        int mana = players[my_camp].mana;
        auto deck = players[my_camp].creature_capacity;
        ::map<string, int> available_count;

        for (const auto &card_unit:deck)
            available_count[card_unit.type] = card_unit.available_count;

            auto pos=available_summon_pos_list[0];

            if (available_count["Priest"] > 0&&mana>=4) {
                summon("Priest",2,pos);
                suc = true;
                continue;
            }
            if (available_count["Swordsman"] > 0 && mana >=4) {
                summon("Swordsman",2,pos);
                suc = true;
                continue;
            }
            if(available_count["Swordsman"] > 0 && mana >=2){
                summon("Swordsman",1,pos);
                suc=true;
                continue;
            }
            if(available_count["Archer"] > 0 && mana >=6){
                summon("Archer",3,pos);
                suc=true;
                continue;
            }
            if(available_count["Archer"] > 0 && mana >=4){
                summon("Archer",2,pos);
                suc=true;
                continue;
            }
            if(available_count["Archer"] > 0 && mana >=2){
                summon("Archer",1,pos);
                suc=true;
                continue;
            }
        }
    }

void AI::start(){
    start_attack();
    start_move();
    start_summon();
}

bool cmp_ally2(const Unit &unit1, const Unit &unit2){
    if (unit1.can_atk != unit2.can_atk)  //首先要能动
            return unit2.can_atk < unit1.can_atk;
        else if (unit1.type != unit2.type) {
            auto type_id_gen = [](const string &type_name) {
                if (type_name == "Inferno") return 0;
                else if (type_name == "Swordsman") return 1;
                else if(type_name=="Archer")return 2;
                else return 3;
            };
            return (type_id_gen(unit1.type) < type_id_gen(unit2.type));
        } 
        else{//离阵营近的优先攻击。
            return cube_distance(unit1.pos,center)<cube_distance(unit2.pos,center);
        }
}

void AI::mid_attack(){
    auto ally_list = getUnitsByCamp(camp1);
    auto enemy_list=getUnitsByCamp(camp2);

    //按顺序排列好单位，依次攻击
    sort(ally_list.begin(), ally_list.end(), cmp_ally2);

    for (const auto &ally:ally_list) {
        if (!ally.can_atk) break;
        vector<Unit> target_list;
        for (const auto &enemy : enemy_list)
            if (AiClient::canAttack(ally, enemy))
                target_list.push_back(enemy);
        if(ally.type=="Inferno"&&cube_distance(ally.pos,enemy_center)==1){
            attack(ally.id,camp2);
            continue;
        }
        if (target_list.empty()) continue;

        if (ally.type == "Priest") {
            int j=findbest_priest(target_list,ally_list,ally);
            if(j!=-1)
                attack(ally.id,j);
        } else if (ally.type == "Swordsman") {
            int j=findbest_swordsman(target_list,ally_list,ally);
            if(j!=-1)
                attack(ally.id,j);
        } else if (ally.type == "Archer") {
            int j=findbest_archer(target_list,ally_list,ally);
            if(j!=-1)
                attack(ally.id,j);
        }
        else if(ally.type=="Inferno"){
            int j=findbest_inferno(target_list,ally_list,ally);
            if(j!=-1)
                attack(ally.id,j);
        }
    }
}

Pos AI::mid_sword(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list){
    Pos ret=a.pos;
    reach_pos_list.push_back(a.pos);
    int level=25;
    for(auto pos:reach_pos_list){
        if(pos==c4&&can_sword(a,c4,enemy_list)&&level>1){
            ret=c4;
            level=1;
            continue;
        }
        if(pos==ll3&&can_sword(a,ll3,enemy_list)&&level>2){
            ret=ll3;
            level=2;
            continue;
        }
        if(pos==rr3&&can_sword(a,rr3,enemy_list)&&level>3){
            ret=rr3;
            level=3;
            continue;
        }
        if(pos==side&&can_sword(a,side,enemy_list)&&level>4){
            ret=side;
            level=4;
            continue;
        }
        if(pos==side1&&can_sword(a,side1,enemy_list)&&level>5){
            ret=side1;
            level=5;
            continue;
        }
        if(pos==side2&&can_sword(a,side2,enemy_list)&&level>6){
            ret=side2;
            level=6;
            continue;
        }
        if(pos==side3&&can_sword(a,side3,enemy_list)&&level>7){
            ret=side3;
            level=7;
            continue;
        }
        if(pos==side4&&can_sword(a,side4,enemy_list)&&level>8){
            ret=side4;
            level=8;
            continue;
        }

        if(pos==c3&&can_sword(a,c3,enemy_list)&&level>9){
            ret=c3;
            level=9;
            continue;
        }
        if(pos==rr2&&can_sword(a,rr2,enemy_list)&&level>10){
            ret=rr2;
            level=10;
            continue;
        }
        if(pos==ll2&&can_sword(a,ll2,enemy_list)&&level>11){
            ret=ll2;
            level=11;
            continue;
        }
        if(pos==c2&&can_sword(a,c2,enemy_list)&&level>12){
            ret=c2;
            level=12;
            continue;
        }
        if(pos==r2&&can_sword(a,r2,enemy_list)&&level>13){
            ret=r2;
            level=13;
            continue;
        }
        if(pos==l2&&can_sword(a,l2,enemy_list)&&level>14){
            ret=l2;
            level=14;
            continue;
        }
        if(can_sword(a,pos,enemy_list)&&level>15&&att(pos,enemy_list,a)&&cube_distance(center,pos)>=2&&cube_distance(center,pos)<=5){
            ret=pos;
            level=15;
            continue;
        }
        if(can_sword(a,pos,enemy_list)&&level>16&&att(pos,enemy_list,a)&&cube_distance(center,pos)==6){
            ret=pos;
            level=16;
            continue;
        }
        if(can_sword(a,pos,enemy_list)&&level>17&&att(pos,enemy_list,a)){
            ret=pos;
            level=17;
            continue;
        }
        if(can_sword(a,pos,enemy_list)&&level>18&&cube_distance(center,pos)>=2&&cube_distance(center,pos)<=5){
            ret=pos;
            level=18;
            continue;
        }
        if(can_sword(a,pos,enemy_list)&&level>19&&cube_distance(center,pos)==6){
            ret=pos;
            level=19;
            continue;
        }
        if(can_sword(a,pos,enemy_list)&&level>20){
            ret=pos;
            level=20;
            continue;
        }
    }
    return ret;
}

Pos AI::mid_priest(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list){
    Pos ret=a.pos;
    reach_pos_list.push_back(a.pos);
    int level=15;
    for(auto pos:reach_pos_list){
        if(pos==c3&&can_priest(a,c3,enemy_list)){
            ret=c3;
            level=1;
            continue;
        }
        if(pos==r3&&can_priest(a,r3,enemy_list)&&level>2){
            ret=r3;
            level=2;
            continue;
        }
        if(pos==l3&&can_priest(a,l3,enemy_list)&&level>3){
            ret=l3;
            level=3;
            continue;
        }
        if(pos==c2&&can_priest(a,c2,enemy_list)&&level>4){
            ret=c2;
            level=4;
            continue;
        }
        if(pos==l2&&can_priest(a,l2,enemy_list)&&level>5){
            ret=l2;
            level=5;
            continue;
        }
        if(pos==r2&&can_priest(a,r2,enemy_list)&&level>6){
            ret=r2;
            level=6;
            continue;
        }
        if(can_priest(a,pos,enemy_list)&&level>7&&att(pos,enemy_list,a)&&cube_distance(c1,pos)>=2&&cube_distance(c1,pos)<=3){
            ret=pos;
            level=7;
            continue;
        }
        if(can_priest(a,pos,enemy_list)&&level>8&&cube_distance(c1,pos)>=2&&cube_distance(c1,pos)<=3){
            ret=pos;
            level=8;
            continue;
        }
        if(can_priest(a,pos,enemy_list)&&level>9&&att(pos,enemy_list,a)&&cube_distance(c1,pos)==4){
            ret=pos;
            level=9;
            continue;
        }
        if(can_priest(a,pos,enemy_list)&&level>10&&att(pos,enemy_list,a)){
            ret=pos;
            level=10;
            continue;
        }
        if(can_priest(a,pos,enemy_list)&&level>11&&cube_distance(c1,pos)==4){
            ret=pos;
            level=11;
            continue;
        }
        if(can_priest(a,pos,enemy_list)&&level>12){
            ret=pos;
            level=12;
            continue;
        }
    }
    return ret;
}

void AI::mid_move(){
    auto ally_list = getUnitsByCamp(my_camp);
    sort(ally_list.begin(), ally_list.end(), [](const Unit &_unit1, const Unit &_unit2) {
        auto type_id_gen = [](const string &type_name) {
            if (type_name == "Inferno") return 0;
            else if (type_name == "Swordsman") return 1;
            else if (type_name=="Archer") return 2;
            else return 3;
        };
        return type_id_gen(_unit1.type) < type_id_gen(_unit2.type);
    });
    for (const auto &ally : ally_list) {
        if (!ally.can_move) continue;
        auto reach_pos_with_dis = reachable(ally, map);
            vector<Pos> reach_pos_list;
            for (const auto &reach_pos : reach_pos_with_dis) {
                for (auto pos : reach_pos)
                    reach_pos_list.push_back(pos);
            }
            if (reach_pos_list.empty()) continue;
        auto enemy_list=getUnitsByCamp(camp2);

        if (ally.type == "Swordsman") {
            Pos ps=start_sword(ally,reach_pos_list,enemy_list);
            if(ps==ally.pos)
                continue;
            move(ally.id, ps);
        } 
        else if(ally.type=="Priest"){
           Pos ps=start_priest(ally,reach_pos_list,enemy_list);
            if(ps==ally.pos)
                continue;
            move(ally.id, ps);
        }
        else if(ally.type=="Archer"){
            Pos ps=start_archer(ally,reach_pos_list,enemy_list);
            if(ps==ally.pos)
                continue;
            move(ally.id, ps);
        }
        else if(ally.type=="Inferno"){
            Pos ps=mid_inferno(ally,reach_pos_list,enemy_list);
            if(ps==ally.pos)
                continue;
            move(ally.id, ps);
        }
    }
}

bool AI::use_artifact(Unit a){
    vector<Pos> all;
        for(int i=0;i<=5;i++)
            all.push_back(cube_neighbor(a.pos,i));
    sort(all.begin(), all.end(), [this](Pos _pos1, Pos _pos2) {
            return cube_distance(_pos1,center) < cube_distance(_pos2,center);
        });
    vector<Pos> pos_list;
    for(auto pos:all){
        if(canUseArtifact(players[camp1].artifact[0],pos,camp1)){
            pos_list.push_back(pos);
        }
    }
    if(pos_list.empty())return 0;
    
         auto best_pos = pos_list[0];
          int max_benefit = 0;
           for (auto pos:pos_list) {
              auto unit_list = units_in_range(pos, 2, map,camp2);
              if (unit_list.size() > max_benefit) {
                  best_pos = pos;
                  max_benefit = unit_list.size();
                }
            }
    use(players[camp1].artifact[0].id,best_pos);
    return 1;
}

bool AI::use_artifact(){
    vector<Pos> all=all_pos_in_map();
    sort(all.begin(), all.end(), [this](Pos _pos1, Pos _pos2) {
            return cube_distance(_pos1,center) < cube_distance(_pos2,center);
        });
    vector<Pos> pos_list;
    for(auto pos:all){
        if(canUseArtifact(players[camp1].artifact[0],pos,camp1)){
            pos_list.push_back(pos);
        }
    }
    if(pos_list.empty())return 0;
    
         auto best_pos = pos_list[0];
         bool can_use=0;
          int max_benefit = 3;
           for (auto pos:pos_list) {
              auto unit_list = units_in_range(pos, 2, map,camp2);
              if (unit_list.size() > max_benefit) {
                  can_use=1;
                  best_pos = pos;
                  max_benefit = unit_list.size();
                }
            }
    if(can_use)
       use(players[camp1].artifact[0].id,best_pos);
    return 1;
}
void AI::mid_artifact(){
    if(players[camp1].max_mana>=8||players[camp1].artifact[0].state == "Ready"){
        auto enemy_list=getUnitsByCamp(camp2);
        Unit target;
        int level=10;
        for(auto enemy:enemy_list){
           int dis = cube_distance(enemy.pos,center);
            if (enemy.atk_range[0] <= dis && dis <= enemy.atk_range[1]){
                bool suc=use_artifact(enemy);
                if(suc)
                    return;
            }
        }
        for(auto enemy:enemy_list){
            if (enemy.type=="Inferno"){
                bool suc=use_artifact(enemy);
                if(suc)return;
            }
        }
        if(checkBarrack(side)==camp2&&getUnitByPos(side,0).id==-1){
                use(players[camp1].artifact[0].id,side);
                return;
            }
            if(checkBarrack(side)==camp2&&getUnitByPos(side2,0).id==-1){
                use(players[camp1].artifact[0].id,side2);
                return;
            }
            if(checkBarrack(side)==camp2&&getUnitByPos(side1,0).id==-1){
                use(players[camp1].artifact[0].id,side1);
                return;
            }
            if(checkBarrack(side)==camp2&&getUnitByPos(side4,0).id==-1){
                use(players[camp1].artifact[0].id,side4);
                return;
            }
        for(auto enemy:enemy_list){
            if (enemy.holy_shield){
                bool suc=use_artifact(enemy);
                if(suc)
                    return;
            }
        }
        use_artifact();
    }
}

void AI::mid_summon(){
    bool suc=true;
    while(suc){
        suc=false;
        vector<Pos> summon_pos_list = getSummonPosByCamp(camp1);
        sort(summon_pos_list.begin(), summon_pos_list.end(), [this](Pos _pos1, Pos _pos2) {
            return cube_distance(_pos1,center) < cube_distance(_pos2,center);
        });
        vector<Pos> available_summon_pos_list;
        for (auto pos:summon_pos_list) {
            auto unit_on_pos_ground = getUnitByPos(pos, false);
            if (unit_on_pos_ground.id == -1) available_summon_pos_list.push_back(pos);
        }
        if(available_summon_pos_list.empty())return;

        int mana = players[my_camp].mana;
        auto deck = players[my_camp].creature_capacity;
        ::map<string, int> available_count;

        for (const auto &card_unit:deck)
            available_count[card_unit.type] = card_unit.available_count;

            auto pos=available_summon_pos_list[0];
             if (available_count["Archer"] > 0&&mana>=6&&getdamageex(pos,"Archer")<4) {
                summon("Archer",3,pos);
                suc = true;
                continue;
            }
            if(available_count["Priest"] > 0 && mana >=7&&getdamageex(pos,"BlackBat")<6){
                summon("Priest",3,pos);
                suc=true;
                continue;
            }
            if(available_count["Priest"] > 0 && mana >=4&&getdamageex(pos,"BlackBat")<4){
                summon("Priest",2,pos);
                suc=true;
                continue;
            }
            if(available_count["Swordsman"] > 0 && mana >=4&&getdamage(pos,"Archer")+getdamage(pos,"BlackBat")<4){
                summon("Swordsman",2,pos);
                suc=true;
                continue;
            }
            if(available_count["Swordsman"] > 0 && mana >=2&&getdamage(pos,"Archer")+getdamage(pos,"BlackBat")<2){
                summon("Swordsman",1,pos);
                suc=true;
                continue;
            }
           
            if(available_count["Swordsman"] > 0 && mana >=6){
                summon("Swordsman",3,pos);
                suc=true;
                continue;
            }
        }
    }
void AI::mid(){
    mid_attack();
    mid_move();
    mid_artifact();
    mid_summon();
}


Pos AI::last1_priest(Unit a,vector<Pos> reach_pos_list,vector<Unit> enemy_list){
    Pos ret=a.pos;
    reach_pos_list.push_back(a.pos);
    int level=15;
    for(auto pos:reach_pos_list){
        if(pos==c4&&can_priest(a,c4,enemy_list)){
            ret=c4;
            level=1;
            continue;
        }
        if(pos==l4&&can_priest(a,l4,enemy_list)&&level>2){
            ret=l4;
            level=2;
            continue;
        }
        if(pos==r4&&can_priest(a,r4,enemy_list)&&level>3){
            ret=l3;
            level=3;
            continue;
        }
        if(pos==c3&&can_priest(a,c3,enemy_list)&&level>4){
            ret=c3;
            level=4;
            continue;
        }
        if(pos==r3&&can_priest(a,r3,enemy_list)&&level>5){
            ret=r3;
            level=5;
            continue;
        }
        if(pos==l3&&can_priest(a,l3,enemy_list)&&level>6){
            ret=l3;
            level=6;
            continue;
        }
        if(pos==c2&&can_priest(a,c2,enemy_list)&&level>7){
            ret=c2;
            level=7;
            continue;
        }
        if(can_priest(a,pos,enemy_list)&&level>8&&att(pos,enemy_list,a)&&cube_distance(c1,pos)>=2&&cube_distance(c1,pos)<=3){
            ret=pos;
            level=8;
            continue;
        }
        if(can_priest(a,pos,enemy_list)&&level>9&&cube_distance(c1,pos)>=2&&cube_distance(c1,pos)<=3){
            ret=pos;
            level=9;
            continue;
        }
        if(can_priest(a,pos,enemy_list)&&level>10&&att(pos,enemy_list,a)&&cube_distance(c1,pos)==4){
            ret=pos;
            level=10;
            continue;
        }
        if(can_priest(a,pos,enemy_list)&&level>11&&att(pos,enemy_list,a)){
            ret=pos;
            level=11;
            continue;
        }
        if(can_priest(a,pos,enemy_list)&&level>12&&cube_distance(c1,pos)==4){
            ret=pos;
            level=12;
            continue;
        }
        if(can_priest(a,pos,enemy_list)&&level>13){
            ret=pos;
            level=13;
            continue;
        }
    }
    return ret;
}

void AI::last1_move(){
    auto ally_list = getUnitsByCamp(my_camp);
    sort(ally_list.begin(), ally_list.end(), [](const Unit &_unit1, const Unit &_unit2) {
        auto type_id_gen = [](const string &type_name) {
            if (type_name == "Inferno") return 0;
            else if (type_name == "Swordsman") return 1;
            else if (type_name=="Archer") return 2;
            else return 3;
        };
        return type_id_gen(_unit1.type) < type_id_gen(_unit2.type);
    });
    for (const auto &ally : ally_list) {
        if (!ally.can_move) continue;
        auto reach_pos_with_dis = reachable(ally, map);
            vector<Pos> reach_pos_list;
            for (const auto &reach_pos : reach_pos_with_dis) {
                for (auto pos : reach_pos)
                    reach_pos_list.push_back(pos);
            }
            if (reach_pos_list.empty()) continue;
        auto enemy_list=getUnitsByCamp(camp2);

        if (ally.type == "Swordsman") {
            Pos ps=start_sword(ally,reach_pos_list,enemy_list);
            if(ps==ally.pos)
                continue;
            move(ally.id, ps);
        } 
        else if(ally.type=="Priest"){
           Pos ps=start_priest(ally,reach_pos_list,enemy_list);
            if(ps==ally.pos)
                continue;
            move(ally.id, ps);
        }
        else if(ally.type=="Archer"){
            Pos ps=start_archer(ally,reach_pos_list,enemy_list);
            if(ps==ally.pos)
                continue;
            move(ally.id, ps);
        }
        else if(ally.type=="Inferno"){
            Pos ps=mid_inferno(ally,reach_pos_list,enemy_list);
            if(ps==ally.pos)
                continue;
            move(ally.id, ps);
        }
    }
}


void AI::last1(){
    mid_attack();
    last1_move();
    mid_artifact();
    mid_summon();
}


void AI::play()
{
    //玩家需要编写的ai操作函数

    /*
    本AI采用这样的策略：
    在首回合进行初期设置、在神迹优势路侧前方的出兵点召唤一个1星弓箭手
    接下来的每回合，首先尽可能使用神器，接着执行生物的战斗，然后对于没有进行战斗的生物，执行移动，最后进行召唤
    在费用较低时尽可能召唤星级为1的兵，优先度剑士>弓箭手>火山龙
    【进阶】可以对局面进行评估，优化神器的使用时机、调整每个生物行动的顺序、调整召唤的位置和生物种类、星级等
    */

    if (round == 0 || round == 1) {
        //先确定自己的基地、对方的基地
        
        camp1=my_camp;
        camp2=camp1^1;
        if(camp1==0)
            K=1;
        else 
            K=-1;
        
        enemy_center={7*K,-7*K,0};
        center={-7*K,7*K,0};
        side={0,5*K,-5*K};
        side1={0,4*K,-4*K};
        side2={-1*K,5*K,-4*K};
        side3={1*K,4*K,-5*K};
        side4={-1*K,6*K,-5*K};
        ll1={-8*K,6*K,2*K};
        ll2={-7*K,5*K,2*K};
        ll3={-6*K,4*K,2*K};
        ll4={-5*K,3*K,2*K};
        l1={-7*K,6*K,1*K};
        l2={-6*K,5*K,1*K};
        l3={-5*K,4*K,1*K};
        l4={-4*K,3*K,1*K};
        c1={-6*K,6*K,0};
        c2={-5*K,5*K,0};
        c3={-4*K,4*K,0};
        c4={-3*K,3*K,0};
        r1={-6*K,7*K,-1*K};
        r2={-5*K,6*K,-1*K};
        r3={-4*K,5*K,-1*K};
        r4={-3*K,4*K,-1*K};
        rr1={-6*K,8*K,-2*K};
        rr2={-5*K,7*K,-2*K};
        rr3={-4*K,6*K,-2*K};
        rr4={-3*K,5*K,-2*K};
        lll1={-8*K,5*K,3*K};
        lll2={-7*K,4*K,3*K};
        lll3={-6*K,3*K,3*K};
        rrr1={-5*K,8*K,-3*K};
        rrr2={-4*K,7*K,-3*K};
        rrr3={-3*K,6*K,-3*K};

        miracle_pos = map.miracles[camp1].pos;

        // 在正中心召唤一个剑士，用来抢占驻扎点
        summon("Archer", 1, c1);
    }
    else if(round == 2 || round == 3){
        gameunit::Unit unit1=getUnitByPos(c1,0);
        int id1=unit1.id;
        move(id1,rr4);
        summon("Archer",1,c1);

    } 
    else if(round == 4 || round == 5){
        gameunit::Unit unit1=getUnitByPos(rr4,0);
        gameunit::Unit unit2=getUnitByPos(c1,0);
        int id1=unit1.id;
        int id2=unit2.id;
        move(id1,side);
        move(id2,c4);
        summon("Archer",1,c1);

    } 
    else if(round == 6 || round == 7){
        gameunit::Unit unit1=getUnitByPos(side,0);
        int id1=unit1.id;
        move(id1,rrr3);
        start_attack();
        if(players[camp1].max_mana>=4)
            summon("Priest",2,r1);
        else
            summon("Priest",1,r1);
        

    }
    else if(round<50&&players[camp1].max_mana<7){
        start();
    }
    else if(round<50&&players[camp1].max_mana>=7){
        mid();
    }
    else{
        last1();
    }

    endRound();
}

int main()
{
    std::ifstream datafile("Data.json");  
    json all_data;
    datafile>>all_data;
    card::get_data_from_json(all_data);

    AI player_ai;
    player_ai.chooseCards();
    while (true) {
        player_ai.updateGameInfo();
        player_ai.play();
    }
    return 0;
}
