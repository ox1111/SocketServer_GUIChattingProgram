﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SharpDX;
using SharpDX.Direct3D9;

namespace JIGAPClientDXGUI.Engine
{
    class texture : IDisposable
    {
        public Texture d3dTex { get; private set; } = null;
        public ImageInformation d3dInfo { get; private set; } = new ImageInformation();

        public texture(Texture inTex, ImageInformation inInfo)
        {
            d3dTex = inTex;
            d3dInfo = inInfo;
        }

        public void Draw()
        {
            if (d3dTex != null)
                DXManager.GetInst().d3dSprite.Draw(d3dTex, Color.White);

        }

        public void Draw(Color color)
        {
            if (d3dTex != null)
                DXManager.GetInst().d3dSprite.Draw(d3dTex, color);

        }


        public void Dispose()
        {
            d3dTex.Dispose();
        }
    }



    class ImageManager : IDisposable
    {
        private static ImageManager Instance = null;
        public static ImageManager GetInst()
        {
            if (Instance == null)
                Instance = new ImageManager();

            return Instance;
        }

        Dictionary<string, texture> textures = new Dictionary<string, texture>();

        public texture LoadTexture(string key, string path = "None")
        {
            texture myTex = null;
          
            if (!textures.TryGetValue(key, out myTex))
            { 
                ImageInformation info;

                Texture d3dTex = Texture.FromFile(DXManager.GetInst().d3dDevice, path,
                D3DX.DefaultNonPowerOf2, D3DX.DefaultNonPowerOf2, 1, Usage.None, Format.Unknown, Pool.Managed, Filter.None, Filter.None, 0, out info);

                myTex = new texture(d3dTex, info);

                textures.Add(key, myTex);
            }

            return myTex;
        }

        public void Dispose()
        {
            foreach(KeyValuePair<string, texture> tex in textures)
                tex.Value.Dispose();

            for (int i = 0; i < textures.ToList().Count(); ++i)
                textures.ToList()[i].Value.Dispose();

            textures.Clear();
            GC.SuppressFinalize(true);
        }
    }

}