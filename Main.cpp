# include <Siv3D.hpp>

void Main()
{
	// ウィンドウを 1280x720 にリサイズ
	Window::Resize(1280, 720);

	// 2D 物理演算のシミュレーションステップ（秒）
	constexpr double stepSec = (1.0 / 60.0);

	// 2D 物理演算のシミュレーション蓄積時間（秒）
	double accumulatorSec = 0.0;

	// 2D 物理演算のワールド
	P2World world{ 980 };

	auto ground = world.createLine(
		P2Static,
		Vec2{ 0,0 },
		Line{ Vec2{-300,0},Vec2{300,0} },
		OneSided::No,
		P2Material{},
		P2Filter{ .categoryBits = 0b0000'0000'0000'0001, .maskBits = 0b1111'1111'1111'1111 });

	auto body = world.createPlaceholder(P2Dynamic, Vec2{ 0,-100 });
	{
		body.addPolygon(
			RoundRect{ -32,-128,64,128,32 }.asPolygon(4),
			P2Material{},
			P2Filter{ .categoryBits = 0b0000'0000'0000'0010, .maskBits = 0b1111'1111'1111'1111 });
		body.setFixedRotation(true);
	}

	auto weapon = world.createPlaceholder(P2Dynamic, Vec2{ 0,-164 });
	{
		// 地面とプレイヤーは接触しないようにする
		P2Filter filter = P2Filter{ .categoryBits = 0b0000'0000'0000'0100, .maskBits = 0b1111'1111'1111'1100 };
		//武器の本体部分
		{
			weapon.addCircleSensor(Circle{ 0,-50,10 }, filter);
			weapon.addCircleSensor(Circle{ 0,-70,10 }, filter);
			weapon.addCircleSensor(Circle{ 0,-90,10 }, filter);
			weapon.addCircleSensor(Circle{ 0,-110,10 }, filter);
			weapon.addCircleSensor(Circle{ 0,-130,10 }, filter);
		}
		weapon.setFixedRotation(true);
		weapon.setAngle(-20_deg);
	}

	auto joint = world.createPivotJoint(weapon, body, Vec2{ 0,-164 });

	Camera2D camera{ Vec2{0,-100} };

	constexpr ColorF WireframeColor{ 0.0,1.0,0.7 };

	Timer timer{ 0.25s };
	double direction = 1;
	bool isAttack{ false };

	while (System::Update())
	{
		ClearPrint();

		for (accumulatorSec += Scene::DeltaTime(); stepSec <= accumulatorSec; accumulatorSec -= stepSec)
		{
			// 2D 物理演算のワールドを更新
			world.update(stepSec);
		}

		// 移動方向のスケール
		double scale = KeyD.pressed() - KeyA.pressed();

		// 攻撃していないときは移動を行う
		if (not isAttack)
		{
			// 武器の初期回転値をプレイヤーがどちらを向いているかによって更新
			if (scale > 0)
			{
				weapon.setAngle(-Abs(weapon.getAngle()));
				direction = scale;
			}
			else if (scale < 0)
			{
				weapon.setAngle(Abs(weapon.getAngle()));
				direction = scale;
			}

			body.applyForce(Vec2{ scale * 300, 0 });
		}

		// 攻撃
		if (KeyEnter.down())
		{
			timer.restart();
			weapon.setAngularVelocity(600_deg * direction);
			isAttack = true;
		}

		// タイマーが終わった
		if (timer.progress0_1() == 1.0)
		{
			// 武器を初期の回転に戻す
			weapon.setAngle(20_deg * -direction);
			weapon.setAngularVelocity(0);
			timer.reset();
			isAttack = false;
		}

		// 描画
		{
			auto tf = camera.createTransformer();

			ground.drawWireframe(1.0, WireframeColor);

			body.drawWireframe(1.0, WireframeColor);

			weapon.drawWireframe(1.0, WireframeColor);

			joint.draw(WireframeColor);
		}

		camera.draw(Palette::Orange);
	}
}
