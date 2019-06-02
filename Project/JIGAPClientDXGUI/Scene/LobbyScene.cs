﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using JIGAPClientDXGUI.Engine;

namespace JIGAPClientDXGUI
{
    class LobbyScene : Scene
    {
        public override void Init()
        {
            ObjectManager.GetInst().AddObject<LobbyLauncher>();

        }

        public override void Release()
        {
            ObjectManager.GetInst().ClearObjects();
        }

        public override void Update()
        {
            ObjectManager.GetInst().Update();
        }

        public override void Render()
        {
            ObjectManager.GetInst().Render();
        }
    }
}