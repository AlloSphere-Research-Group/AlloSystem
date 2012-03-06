#ifndef AL_UTIL_VOXEL_STACK_HPP
#define AL_UTIL_VOXEL_STACK_HPP

#include "allocore/math/al_Functions.hpp"

#include <list>
#include <vector>

namespace al {
	
	template<typename T>
	struct VoxelStack {

		template<int N>
		struct Stack {

			Stack() { nodes.reserve(N); }
			Stack(const Stack& cpy) { nodes.reserve(N); }

			void push(T * p) {
				if (nodes.size() < N) {
					nodes.push_back(p);
				}
			}

			void clear() {
				nodes.clear();
			}

			T * top() const {
				if (!nodes.empty()) {
					return nodes.back();
				} else {
					return NULL;
				}
			}

			T * pop() {
				if (!nodes.empty()) {
					T * v = nodes.back();
					nodes.pop_back();
					return v;
				}
				return NULL;
			}

			// would use std::stack, but it doesn't have clear().
			std::vector<T *> nodes;
		};

		VoxelStack(int dim=32) : mDim(al::ceilPow2(dim)), mDim2(mDim*mDim), mDim3(mDim*mDim*mDim), mDimWrap(mDim-1) {
			voxels.resize(mDim3);
		}

		void clear() {
			for (int i=0; i<mDim3; i++){
				voxels[i].clear();
			}
		}

		inline int pos2voxelindex(const Vec3d & v) {	// column-major.
			return (unsigned(int(v[0])) & mDimWrap)
				 + (unsigned(int(v[1])) & mDimWrap)*mDim
				 + (unsigned(int(v[2])) & mDimWrap)*mDim2;
		}

		void set(const Vec3d& pos, T * v) {
			set(pos2voxelindex(pos), v);
		}
		T * at(const Vec3d& pos) const {
			//return voxels[pos2voxelindex(pos)];
			return at(pos2voxelindex(pos));
		}
		T * pop(const Vec3d& pos) {
//			int i = pos2voxelindex(pos);
//			T * v = voxels[i];
//			voxels[i] = NULL;
//			return v;
//			int i = pos2voxelindex(pos);
//			T * v = voxels[i].front();
//			voxels[i].pop_front();
//			return v;
			return pop(pos2voxelindex(pos));
		}

		void set(int i, T * v) {
			voxels[i].push(v);
		}
		T * at(int i) const {
			return voxels[i].top();
		}
		T * pop(int i) {
			//T * v = voxels[i].front();
			T * v = voxels[i].pop(); //pop_front();
			return v;
		}

		std::list<T *>& operator[](int i) { return voxels[i]; }
		const std::list<T *>& operator[](int i) const { return voxels[i]; }

	protected:
		//std::vector< std::list< T * > > voxels;
		std::vector<Stack<8> > voxels;
		
		unsigned mDim, mDim2, mDim3, mDimWrap;
	};

} // al::

#endif