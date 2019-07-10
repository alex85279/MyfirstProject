#include <iostream>
#include <vector>
#include <STcpClient.h>
using namespace std;
int x_offset[] = { -1,1,0,0 };
int y_offset[] = { 0,0,-1,1 };
class Coordinate {
public:
	int x;
	int y;
	Coordinate(int x, int y) {
		this->x = x;
		this->y = y;
	}
	bool IsInBoard() {
		return !(this->x < 0 || this->x > 7 || this->y < 0 || this->y > 7);
	}
	bool isOnUpperEdge() {
		return (this->x == 0);
	}
	bool isOnDownEdge() {
		return (this->x == 7);
	}
};
class Step_reward {
public:
	vector<Coordinate> steps;
	int reward;
	bool movable;
	Step_reward(int reward, bool movable) : reward(reward), movable(movable) {};
	Step_reward(Coordinate pawn, int reward, bool movable) {
		this->steps.push_back(pawn);
		this->reward = reward;
		this->movable = movable;
	}
};
int ForwardReward(Coordinate old_pawn, Coordinate new_pawn, bool is_black);
Step_reward GetJumpStep(vector<vector<int>> tmp_board, Coordinate pawn, bool is_black, int layer);
int GetStepValue(vector<vector<int>> & board, bool is_black, int layer);
void InitializeBoard(vector<vector<int>> & board) {
	int myints0[] = { 1, 0, 0, 0, 0, 0, 0, 0 };
	int myints1[] = { 0, 1, 0, 0, 0, 0, 0, 2 };
	int myints2[] = { 1, 0, 1, 0, 0, 0, 2, 0 };
	int myints3[] = { 0, 1, 0, 0, 0, 2, 0, 2 };
	int myints4[] = { 1, 0, 1, 0, 0, 0, 2, 0 };
	int myints5[] = { 0, 1, 0, 0, 0, 2, 0, 2 };
	int myints6[] = { 1, 0, 0, 0, 0, 0, 2, 0 };
	int myints7[] = { 0, 0, 0, 0, 0, 0, 0, 2 };
	board.resize(8);
	board[0].assign(myints0, myints0 + 8);
	board[1].assign(myints1, myints1 + 8);
	board[2].assign(myints2, myints2 + 8);
	board[3].assign(myints3, myints3 + 8);
	board[4].assign(myints4, myints4 + 8);
	board[5].assign(myints5, myints5 + 8);
	board[6].assign(myints6, myints6 + 8);
	board[7].assign(myints7, myints7 + 8);
	return;
}
void PrintBoard(std::vector<std::vector<int>> & board) {
	printf("~ 0 1 2 3 4 5 6 7\n");
	for (int n = 0; n < 8; n++) {
		cout << n << " ";
		for (int m = 0; m < 8; m++) {
			switch (board[n][m]) {
			case 0:
				cout << "- ";
				break;
			case 1:
				cout << "X ";
				break;
			case 2:
				cout << "O ";
				break;
			}
		}
		printf("\n");
	}
	printf("\n");
}
void GetPawns(std::vector<vector<int>> & board, vector<Coordinate> & pawns, bool is_black) {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (is_black && board[i][j] == 1)
				pawns.emplace_back(i, j);
			else if (!is_black && board[i][j] == 2)
				pawns.emplace_back(i, j);
		}
	}
}
void PrintPawns(vector<Coordinate> & pawns) {
	for (int n = 0; n < pawns.size(); n++)
		printf("%d %d\n", pawns[n].x, pawns[n].y);
}

int field_evaluation(vector<vector<int>> tmp_board, Coordinate pawn, bool is_black) {
	int field_reward = 0;
	for (int n = 0; n < 4; n++) {
		Coordinate new_pawn_pos(pawn.x + x_offset[n], pawn.y + y_offset[n]);
		if (!new_pawn_pos.IsInBoard())
			continue;
		if (tmp_board[new_pawn_pos.x][new_pawn_pos.y] == ((is_black) ? 1 : 2))
			field_reward += 1;
	}
	
	vector<Coordinate> pawns;
	vector<Coordinate> enemys;
	int upper_pawn_count = 0;
	int down_pawn_count = 0;
	int upper1_idx = -1;
	int upper2_idx = -1;
	int down1_idx = -1;
	int down2_idx = -1;
	int pawn_x[8] = { 0,0,0,0,0,0,0,0 };
	GetPawns(tmp_board, pawns, is_black);
	GetPawns(tmp_board, enemys, !is_black);
	int inside = 0;
	int enemy_inside = 0;
	for (int i = 0; i < pawns.size(); i++) {
		if (pawns[i].isOnDownEdge()) {
			if (down_pawn_count == 0)
				down1_idx = i;
			else if (down_pawn_count == 1)
				down2_idx = i;
			down_pawn_count++;
		}
		if (pawns[i].isOnUpperEdge()) {
			if (upper_pawn_count == 0)
				upper1_idx = i;
			else if (upper_pawn_count == 1)
				upper2_idx = i;
			upper_pawn_count++;
		}
		if ((pawns[i].y == 6 || pawns[i].y == 7) && is_black) {
			inside++;
		}
		else if((pawns[i].y == 0 || pawns[i].y == 1) && !is_black) {
			inside++;
		}
	}
	for (int i = 0; i < enemys.size(); i++) {
		if (is_black && enemys[i].y == 0 || enemys[i].y == 1) {
			enemy_inside++;
		}
		else if (!is_black && enemys[i].y == 6 || enemys[i].y == 7) {
			enemy_inside++;
		}
	}
	if (enemy_inside > inside && enemy_inside == enemys.size()) {
		field_reward -= 1000;
	}
	/*if (inside == pawns.size()) {
		field_reward += 10000;
	}
	else {
		field_reward += 30 * inside;
	}*/
	//field_reward -= abs((pawn_x[0] + pawn_x[1] + pawn_x[2] + pawn_x[3]) - (pawn_x[4] + pawn_x[5] + pawn_x[6] + pawn_x[7]));
	if (down_pawn_count > 2)
		field_reward -= 3;
	else if (down_pawn_count == 2) {
		field_reward += 6;
		//field_reward -= abs(pawns[down1_idx].y - pawns[down2_idx].y)*4;
	}
	else if (down_pawn_count == 1)
		field_reward += 3;
	else
		field_reward -= 3;
	if (upper_pawn_count > 2)
		field_reward -= 3;
	else if (upper_pawn_count == 2) {
		field_reward += 6;
		//field_reward -= abs(pawns[upper1_idx].y - pawns[upper2_idx].y)*4;
	}
	else if (upper_pawn_count == 1)
		field_reward += 3;
	else
		field_reward -= 3;
	return field_reward;
}

Step_reward GetPawnStep(vector<vector<int>>& board, Coordinate pawn, bool is_black, int layer) {
	int max_reward = -10000;
	int max_index = -1;
	int enemy = (is_black) ? 2 : 1;
	int player = (is_black) ? 1 : 2;
	Step_reward pawn_step(0, 0);
	for (int n = 0; n < 4; n++) {
		Coordinate new_pawn_pos(pawn.x + x_offset[n], pawn.y + y_offset[n]);
		if (!new_pawn_pos.IsInBoard())
			continue;
		if (board[new_pawn_pos.x][new_pawn_pos.y] == 0) {  //move neighbor
			vector<vector<int>> tmp_board = board;
			tmp_board[pawn.x][pawn.y] = 0;
			tmp_board[new_pawn_pos.x][new_pawn_pos.y] = (is_black) ? 1 : 2;
			int tmp_reward = field_evaluation(tmp_board, new_pawn_pos, is_black);
			int enemy_reward = 0;
			int foward_reward = 0;
			vector<Coordinate> enemy_pawns;
			GetPawns(tmp_board, enemy_pawns, !is_black);
			if (layer == 1 && enemy_pawns.size() > 0) {
				enemy_reward = GetStepValue(tmp_board, !is_black, layer + 1);
				foward_reward = ForwardReward(pawn, new_pawn_pos, is_black);
			}
			else if (layer == 1)
				foward_reward = ForwardReward(pawn, new_pawn_pos, is_black);
			if (tmp_reward - enemy_reward + foward_reward > max_reward) {
				max_reward = tmp_reward - enemy_reward + foward_reward;
				max_index = n;
			}
		}
	}
	//  jump
	vector<vector<int>> tmp_board = board;
	pawn_step = GetJumpStep(tmp_board, pawn, is_black, layer);
	//printf("pawn (%d %d) max reward = %d, jump reward = %d\n", pawn.x, pawn.y, max_reward, pawn_step.reward);

	if (pawn_step.steps.size() > 1) {
		int foward_reward = ForwardReward(pawn_step.steps.back(), pawn_step.steps[0], is_black);
		if (pawn_step.reward + foward_reward > max_reward) {  //return jump step
			//printf("return jump\n");
			return pawn_step;
		}
	}

	Coordinate tmp0(pawn.x, pawn.y);
	Coordinate tmp1(pawn.x + x_offset[max_index], pawn.y + y_offset[max_index]);
	Step_reward walk_reward(max_reward, 1);
	walk_reward.steps.push_back(tmp1);
	walk_reward.steps.push_back(tmp0);
	//printf("test2\n");
	//printf("max index = %d\n", max_index);
	//printf("max_reward = %d\n", pawn_step.reward);
	return walk_reward;
}

int ForwardReward(Coordinate old_pawn, Coordinate new_pawn, bool is_black) {
	int forward_constant = 2;
	int return_reward = 0;
	if (is_black) {
		if (new_pawn.y > old_pawn.y) return_reward += forward_constant;
		else if (new_pawn.y < old_pawn.y)	return_reward -= forward_constant;
	}
	else {
		if (new_pawn.y < old_pawn.y)	return_reward += forward_constant;
		else if (new_pawn.y > old_pawn.y)	return_reward -= forward_constant;
	}
	if (new_pawn.x == 0 || new_pawn.x == 7)	return_reward += 2;
	return return_reward;
}

Step_reward GetJumpStep(vector<vector<int>> tmp_board, Coordinate pawn, bool is_black, int layer) {
	if (pawn.x < 0 || pawn.x > 7 || pawn.y < 0 || pawn.y > 7) { //out of board
		Step_reward tmp(-1, 0);
		return tmp;
	}
	if (tmp_board[pawn.x][pawn.y] == -1) { // repeat terminate
		Step_reward tmp(pawn, 0, 1);
		tmp.reward = field_evaluation(tmp_board, pawn, is_black);
		return tmp;
	}
	int enemy = (is_black) ? 2 : 1;
	int player = (is_black) ? 1 : 2;
	int max_reward = -10000;
	vector<Coordinate> tmp_steps;
	Step_reward max_step_reward(0, 0);
	max_step_reward.reward = field_evaluation(tmp_board, pawn, is_black);
	for (int n = 0; n < 4; n++) {
		Coordinate new_pawn_pos(pawn.x + x_offset[n] * 2, pawn.y + y_offset[n] * 2);
		Coordinate tool_pawn_pos(pawn.x + x_offset[n], pawn.y + y_offset[n]);
		if (!new_pawn_pos.IsInBoard() || !tool_pawn_pos.IsInBoard())
			continue;
		if (tmp_board[new_pawn_pos.x][new_pawn_pos.y] != 0)
			continue;
		if (tmp_board[tool_pawn_pos.x][tool_pawn_pos.y] == 0)
			continue;
		vector<vector<int>> new_tmp_board = tmp_board;
		//  update new board
		new_tmp_board[pawn.x][pawn.y] = -1; // label -1 for repeat
		new_tmp_board[new_pawn_pos.x][new_pawn_pos.y] = player;
		bool new_eaten = (tmp_board[tool_pawn_pos.x][tool_pawn_pos.y] == enemy);
		if (new_eaten)
			new_tmp_board[tool_pawn_pos.x][tool_pawn_pos.y] = 0;
		Step_reward tmp_step_reward = GetJumpStep(new_tmp_board, new_pawn_pos, is_black, layer);
		int enemy_reward = 0;
		int foward_reward = 0;
		vector<Coordinate> enemy_pawns;
		GetPawns(tmp_board, enemy_pawns, !is_black);
		if (layer == 1 && enemy_pawns.size() > 0) {
			foward_reward = ForwardReward(pawn, new_pawn_pos, is_black);
			enemy_reward = GetStepValue(new_tmp_board, !is_black, layer + 1);
		}
		if (tmp_step_reward.reward + new_eaten * 100
			- enemy_reward + foward_reward > max_step_reward.reward) {
			max_step_reward = tmp_step_reward;
			max_step_reward.reward = tmp_step_reward.reward + new_eaten * 100
				- enemy_reward + foward_reward;
		}
	}
	max_step_reward.steps.push_back(pawn);
	return max_step_reward;
}

vector<vector<int>> GetStep(vector<vector<int>> & board, bool is_black, int layer) {
	vector<Coordinate> pawns;
	GetPawns(board, pawns, is_black);
	//PrintPawns(pawns);
	Step_reward max_step(-10000, 0);
	Step_reward tmp_steps(0, 0);
	for (int n = 0; n < pawns.size(); n++) {
		tmp_steps = GetPawnStep(board, pawns[n], is_black, layer);
		if (max_step.reward < tmp_steps.reward) {
			max_step = tmp_steps;
		}
	}
	//cout <<"reward: "<< max_step.reward << endl;
	//printf("out of pawn loop\n");
	vector<vector<int>> steps;  //transforn vector Coor to vector vector int
	for (int n = max_step.steps.size() - 1; n >= 0; n--) {
		vector<int> tmp_step(2, 0);
		tmp_step[0] = max_step.steps[n].x;
		tmp_step[1] = max_step.steps[n].y;
		steps.push_back(tmp_step);
	}
	for (int n = 0; n < steps.size(); n++) {
		//printf("(%d %d)\n", steps[n][0], steps[n][1]);
	}
	return steps;
}

int GetStepValue(vector<vector<int>> & board, bool is_black, int layer) {
	vector<Coordinate> pawns;
	GetPawns(board, pawns, is_black);
	//PrintPawns(pawns);
	Step_reward max_step(-10000, 0);
	Step_reward tmp_steps(0, 0);
	for (int n = 0; n < pawns.size(); n++) {
		tmp_steps = GetPawnStep(board, pawns[n], is_black, layer);
		if (max_step.reward < tmp_steps.reward) {
			max_step = tmp_steps;
		}
	}
	return max_step.reward;
}

void step_movement(vector<vector<int>> & board, vector<vector<int>> step, bool is_black) {
	int pawn_X = step[0][0];
	int pawn_Y = step[0][1];
	int player = board[pawn_X][pawn_Y];
	int enemy = (is_black) ? 2 : 1;
	cout << "選到的棋子為: " << player << endl;
	for (int i = 1; i < step.size(); i++) {
		int new_X = step[i][0];
		int new_Y = step[i][1];
		printf("從原本的座標: (%d, %d) 移動到新座標: (%d, %d)\n", pawn_X, pawn_Y, new_X, new_Y);
		//walk
		if (abs(new_X - pawn_X) == 1 && abs(new_Y - pawn_Y) == 0) {
			board[new_X][new_Y] = player;
			board[pawn_X][pawn_Y] = 0;
			pawn_X = new_X;
			pawn_Y = new_Y;
		}
		else if (abs(new_X - pawn_X) == 0 && abs(new_Y - pawn_Y) == 1) {
			board[new_X][new_Y] = player;
			board[pawn_X][pawn_Y] = 0;
			pawn_X = new_X;
			pawn_Y = new_Y;
		}
		else if (abs(new_X - pawn_X) == 2 && abs(new_Y - pawn_Y) == 0) {
			board[new_X][new_Y] = player;
			board[pawn_X][pawn_Y] = 0;
			if (board[(new_X + pawn_X) / 2][pawn_Y] == enemy) {
				board[(new_X + pawn_X) / 2][pawn_Y] = 0;
			}
			pawn_X = new_X;
			pawn_Y = new_Y;
		}
		else if (abs(new_X - pawn_X) == 0 && abs(new_Y - pawn_Y) == 2) {
			board[new_X][new_Y] = player;
			board[pawn_X][pawn_Y] = 0;
			if (board[pawn_X][(new_Y + pawn_Y) / 2] == enemy) {
				board[pawn_X][(new_Y + pawn_Y) / 2] = 0;
			}
			pawn_X = new_X;
			pawn_Y = new_Y;
		}
		cout << "step: " << i << endl;
		PrintBoard(board);
	}
}
void human_move(vector<vector<int>> & board, bool is_black) {
	vector<vector<int>> steps;
	int step_num;
	cout << "輸入要走的步數: ";
	cin >> step_num;
	if (step_num == 0)
		return;
	steps.resize(step_num + 1);
	steps[0].resize(2);
	cout << "輸入要移動的棋子: ";
	cin >> steps[0][0] >> steps[0][1];

	for (int i = 1; i < step_num + 1; i++) {
		steps[i].resize(2);
		cout << "輸入要移動到的座標: ";
		cin >> steps[i][0] >> steps[i][1];
	}
	step_movement(board, steps, !is_black);
}

int main() {
	int id_package;
	bool is_black;
	bool pre_is_black;
	int round = 0;
	vector<vector<int>> board, steps;
	vector<int> initial_step(2, 0);
	while (true) {
		if (GetBoard(id_package, board, is_black)) {
			break;
		}
		steps.clear();
		if (round != 0 && pre_is_black != is_black) {
			round = 0;
		}
		if (round == 0) {
			if (is_black) {
				initial_step[0] = 5;
				initial_step[1] = 1;
				steps.push_back(initial_step);

				initial_step[0] = 6;
				initial_step[1] = 1;
				steps.push_back(initial_step);
			}

			else {
				initial_step[0] = 2;
				initial_step[1] = 6;
				steps.push_back(initial_step);

				initial_step[0] = 1;
				initial_step[1] = 6;
				steps.push_back(initial_step);
			}
			round++;
		}
		else
			steps = GetStep(board, is_black, 1);

		pre_is_black = is_black;
		SendStep(id_package, steps);
	}
	/*steps = GetStep(board, is_black);
	for ( int n = 0; n < steps.size(); n++ ) {
		printf("(%d %d)\n",steps[n][0], steps[n][1]);
	}
	step_movement(board, steps, is_black);*/
	printf("finish!!\n");
	return 0;
}
